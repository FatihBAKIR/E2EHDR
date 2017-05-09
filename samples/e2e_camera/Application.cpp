//
// Created by Mehmet Fatih BAKIR on 13/12/2016.
//

#include "shared_frame_queue.hpp"
#include <vector>
#include <e2e_ff/ffmpeg_wrapper.h>
#include <boost/thread.hpp>
#include <texture.h>
#include <glsl_program.h>
#include <Frame.h>
#include "Application.h"
#include "configuration.h"
#include "calibration.h"
#include <camera_struct.h>
#include <shader_stuff.h>
#include <json.hpp>
#include <Window.h>
#include <functional>
#include <merger.h>
#include <profiler/profiler.h>
#include <GLFW/glfw3.h>
#include <jpeg/jpeg_encode.h>
#include <spdlog/spdlog.h>
#include <imgui_wrapper.h>
#include <gui.h>
#include <imgui.h>
#include <set>
#include <camera_control.h>
#include <future>
#include <thread_pool.h>
#include <boost/variant.hpp>
#include <hdr_encode.hpp>
#include <e2e_uvc/camera.hpp>
#include "app_config.hpp"

using json = nlohmann::json;
using namespace e2e::app;
using namespace std::chrono_literals;

class ApplicationImpl
{
	struct GUI
	{
		e2e::Window w;

		Texture left_tex;
		Texture right_tex;

		e2e::Quad left_quad;
		e2e::Quad right_quad;

		GUI();
	};

	GUI gui;
	e2e::Merger merger{ 1280, 720, 16 };

#if defined(E2E_FFMPEG_CAM)
	camera_struct left_cam;
	camera_struct right_cam;

	camera_control left_cam_ctl;
	camera_control right_cam_ctl;
#elif defined(E2E_UVC_CAM)
	e2e::uvc::context uvc_ctx;
	e2e::uvc::camera left_camera;
	e2e::uvc::camera right_camera;

	e2e::uvc::stream left_cam;
	e2e::uvc::stream right_cam;

	e2e::uvc::camera& left_cam_ctl = left_camera;
	e2e::uvc::camera& right_cam_ctl = right_camera;

	nlohmann::json left_meta;
	nlohmann::json right_meta;
#endif

	e2e::GLSLProgram left_prev_shader;
	e2e::GLSLProgram right_prev_shader;

	//e2e::shared_frames_queue mq = {true, 1280, 720};
	bool is_player_available = false;

	struct
	{
		std::vector<std::pair<std::string, crf>> profs;
		std::vector<const char*> profile_names; // does not own
		void update_names()
		{
			profile_names.resize(profs.size());
			for (int i = 0; i < profs.size(); ++i)
			{
				profile_names[i] = std::get<0>(profs[i]).data();
			}
		}

		crf& get_response(int index)
		{
			return std::get<1>(profs[index]);
		}
		std::string get_name(int index)
		{
			return std::get<0>(profs[index]);
		}

	} profiles;

	const FrameT* left_cur_frame;
	const FrameT* right_cur_frame;

	struct
	{
		float   display_exposure = -5;
		bool    apply_crf = true;
		bool    undistort = true;
	} preview;

	std::map<int, std::function<void()>> key_handlers;
	std::vector<std::function<void(const FrameT&, const FrameT&)>> frame_handlers;

	std::set<int> prev_keys;
	e2e::thread_pool tp;

	int snap_counter = 1;

	bool running = false;

	/*
	 * This function finds the most recent image pair
	 */
	std::pair<FrameT, FrameT> get_frame_pair();

    e2e::x264::hdr_encode encoder;

	struct WaitingRec {};

	struct InRecovery
	{
		std::vector<FrameT> left_frames;
		std::vector<FrameT> right_frames;
		std::vector<std::chrono::milliseconds> times;

		std::chrono::milliseconds cur_exposure = 0ms;
		std::chrono::high_resolution_clock::time_point last_take;
	};

	struct DoneRecovery
	{
		crf left_crf;
		crf right_crf;
	};

	using rec_state = boost::variant<WaitingRec, InRecovery, DoneRecovery>;
	rec_state crf_state;


    float align_x = 0;
    float align_y = 0;

    std::chrono::milliseconds cvt[12] = {
			1000ms,
			500ms,
			250ms,
			125ms,
			67ms,
			33ms,
			20ms,
			17ms,
			10ms,
			4ms,
			2ms,
			1ms
	};

	struct CTL_data
	{
		int exposure_index;
		int profile_index;
	};

	CTL_data left;
	CTL_data right;

	bool is_fullscreen = false;

	void snapshot();

	void reload_shaders();

	void add_keybinding(int key, std::function<void()>&& fun);

	void draw_gui();

	void draw_preview();

	void start_crf_recovery();

	void keys();

	void pause();
	void start();

	void recover_routine(const FrameT& left, const FrameT& right);
	void tex_update_routine(const FrameT& l_frame, const FrameT& r_frame)
	{
		gui.left_tex.create(l_frame.width(), l_frame.height(), l_frame.buffer().data());
		gui.right_tex.create(r_frame.width(), r_frame.height(), r_frame.buffer().data());
	}
	void frame_ptr_routine(const FrameT& l_frame, const FrameT& r_frame)
	{
		left_cur_frame = &l_frame;
		right_cur_frame = &r_frame;
	}

public:
	ApplicationImpl(const std::vector <std::string> &args);

	void Run();
};

ApplicationImpl::GUI::GUI() : w(1280, 720)
{
	left_quad.create();
	right_quad.create();

	left_quad.set_position(-0.5, -0.5);
	left_quad.set_scale_factor(0.5, 0.5);

	right_quad.set_position(0.5, -0.5);
	right_quad.set_scale_factor(0.5, 0.5);

	left_quad.set_texture(left_tex);
	right_quad.set_texture(right_tex);

	unsigned char black[] = { 0 };
	left_tex.create(1, 1, black);
	right_tex.create(1, 1, black);
}

ApplicationImpl::ApplicationImpl(const std::vector<std::string> &args) :
#if defined(E2E_FFMPEG_CAM)
	left_cam(load_camera_conf(args[0])),
	right_cam(load_camera_conf(args[1])),
	left_cam_ctl(left_cam.get_ip()),
	right_cam_ctl(right_cam.get_ip()),
#elif defined(E2E_UVC_CAM)
    left_camera(uvc_ctx.open_camera(uvc_ctx.list_cameras()[0])),
    right_camera(uvc_ctx.open_camera(uvc_ctx.list_cameras()[2])),
    left_cam(left_camera, 1280, 720, 24),
    right_cam(right_camera, 1280, 720, 24),
    left_meta(load_camera_conf(args[0])),
	right_meta(load_camera_conf(args[1])),
#endif
	tp(1),
	encoder("output.h264", 1280, 720)
{
#if defined(E2E_FFMPEG_CAM)
	left.exposure_index = (left_cam.get_config())["exp_code"];
	right.exposure_index = (right_cam.get_config())["exp_code"];
#elif defined(E2E_UVC_CAM)
	left.exposure_index = left_meta["exp_code"];
	right.exposure_index = right_meta["exp_code"];
#endif

	left_cam_ctl.set_iso(4);
	right_cam_ctl.set_iso(4);

	/*left_cam_ctl.set_wb_temp(4000);
	right_cam_ctl.set_wb_temp(4000);*/
    left_cam_ctl.set_auto_wb(true);
    right_cam_ctl.set_auto_wb(true);

    left_cam_ctl.set_shutter_speed(cvt[left.exposure_index]);
    right_cam_ctl.set_shutter_speed(cvt[right.exposure_index]);

	add_keybinding(GLFW_KEY_R, [this] {
		reload_shaders();
	});

	add_keybinding(GLFW_KEY_S, [this] {
		snapshot();
	});

	add_keybinding(GLFW_KEY_ESCAPE, [this] {
		gui.w.ShouldClose(true);
	});

	add_keybinding(GLFW_KEY_ENTER, [this] {
		if (is_fullscreen)
		{
			is_fullscreen = false;
			merger.set_position(0.5, 0.5);
			merger.set_scale_factor(0.5, 0.5);
		}
		else
		{
			is_fullscreen = true;
			merger.set_position(0, 0);
			merger.set_scale_factor(1, 1);
		}
	});

    add_keybinding(GLFW_KEY_RIGHT, [this]{
        align_x += 0.002;
    });

    add_keybinding(GLFW_KEY_LEFT, [this]{
        align_x -= 0.002;
    });

    add_keybinding(GLFW_KEY_UP, [this]{
        align_y += 0.002;
    });

    add_keybinding(GLFW_KEY_DOWN, [this]{
        align_y -= 0.002;
    });

    // load profiles
	profiles.profs = find_profiles();
	profiles.update_names();

#if defined(E2E_FFMPEG_CAM)
	auto l_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), left_cam.get_profile());
	auto r_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), right_cam.get_profile());
#elif defined(E2E_UVC_CAM)
	auto l_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), left_meta["profile"]);
	auto r_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), right_meta["profile"]);
#endif

	left.profile_index = std::distance(profiles.profile_names.begin(), l_prof_iter);
	right.profile_index = std::distance(profiles.profile_names.begin(), r_prof_iter);

	e2e::GUI::getGUI().initialize(gui.w, true);
	reload_shaders();
}

void ApplicationImpl::Run()
{
	GLint max_layers;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);

	std::cout << max_layers << "layersss" << std::endl;

	merger.set_textures(gui.left_tex, gui.right_tex);
	merger.set_position(0.5, 0.5);
	merger.set_scale_factor(0.5, 0.5);

	while (!gui.w.ShouldClose())
	{
		named_profile("Display");

		boost::optional<std::pair<FrameT, FrameT>> frames;
		if (running && !left_cam.get_frame_queue().empty() && !right_cam.get_frame_queue().empty())
		{
			frames = get_frame_pair();
			auto& left = std::get<0>(frames.get());
			auto& right = std::get<1>(frames.get());

			tex_update_routine(left, right);
			frame_ptr_routine(left, right);
			recover_routine(left, right);
		}

		gui.w.StartDraw();

		//gui.w.reset_viewport();


		if (running) // && left_cur_frame && right_cur_frame)
		{
			auto er = glGetError();
			//std::cout << "(" << er << ") " << glewGetErrorString(er) << '\n';
			draw_preview();

            merger.get_undistort_left_shader().setUniformFVar("global_align", { align_x, align_y });
            merger.get_undistort_right_shader().setUniformFVar("global_align", { align_x, align_y });

            merger.draw(gui.w);

            /*auto frames = merger.get_frames();

            e2e::LDRFrame tmod{std::move(frames.tonemapped), 1280, 720};
			e2e::LDRFrame resid{std::move(frames.residual), 1280, 720};

			encoder.encode(tmod, resid);

			if (is_player_available)
			{
				mq.push(tmod, resid);
			}*/

			er = glGetError();
			//std::cout << "(" << er << ") " << glewGetErrorString(er) << '\n';
			assert(er == GL_NONE);
		}
		draw_gui();
		gui.w.EndDraw();

		keys();

		left_cur_frame = nullptr;
		right_cur_frame = nullptr;

		if (frames)
		{
#if defined(E2E_FFMPEG_CAM)
			left_cam.recycle(std::move(std::get<0>(frames.get())));
			right_cam.recycle(std::move(std::get<1>(frames.get())));
#endif
		}
	}
}

std::pair<FrameT, FrameT> ApplicationImpl::get_frame_pair() {
	auto& left_queue = left_cam.get_frame_queue();
	auto& right_queue = right_cam.get_frame_queue();

	auto left_frame = std::move(left_queue.front()); left_queue.pop();
	auto right_frame = std::move(right_queue.front()); right_queue.pop();

	auto calc_diff = [](const auto& f1, const auto& f2)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds> (f1.get_time() - f2.get_time()).count();
	};

	auto diff = calc_diff(left_frame, right_frame);

	//TODO: drop frames if too old

	while (true)
	{
		// right is newer than left and there is newer frames on the left queue
		if (right_frame.get_time() > left_frame.get_time() && !left_queue.empty())
		{
			// calculate the difference between the right frame and the more recent frame in the left queue
			auto di = calc_diff(right_frame, left_queue.front());

			// if the difference shrinks, drop left frame
			if (std::abs(diff) > std::abs(di))
			{
#if defined(E2E_FFMPEG_CAM)
				left_cam.recycle(std::move(left_frame));
#endif
				left_frame = std::move(left_queue.front()); left_queue.pop();
				diff = di;
			}
			else
			{
				break;
			}
		}
		else if (left_frame.get_time() > right_frame.get_time() && !right_queue.empty())
		{
			auto di = calc_diff(left_frame, right_queue.front());
			if (std::abs(diff) > std::abs(di))
			{
#if defined(E2E_FFMPEG_CAM)
				right_cam.recycle(std::move(right_frame));
#endif
				right_frame = std::move(right_queue.front()); right_queue.pop();
				diff = di;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return std::make_pair(std::move(left_frame), std::move(right_frame));
}

void ApplicationImpl::snapshot() {
	if (!left_cur_frame || !right_cur_frame) return;

	std::cout << "SAVING" << '\n';

	e2e::save_jpeg(left_cur_frame->buffer().data(), left_cur_frame->width(), left_cur_frame->height(),
		"snap_210_" + std::to_string(snap_counter) + ".jpg");


	e2e::save_jpeg(right_cur_frame->buffer().data(), right_cur_frame->width(), right_cur_frame->height(),
		"snap_110_" + std::to_string(snap_counter) + ".jpg");

	snap_counter++;
}

void ApplicationImpl::reload_shaders() {
	std::cout << "reloading shaders...\n";

	left_prev_shader = make_preview_shader(left_meta);
	right_prev_shader = make_preview_shader(right_meta);

	gui.left_quad.set_program(left_prev_shader);
	gui.right_quad.set_program(right_prev_shader);

	make_merge_shader(merger.get_merge_shader(), left_meta, right_meta);

    make_undistort_shader(merger.get_undistort_left_shader(), left_meta);
    make_undistort_shader(merger.get_undistort_right_shader(), right_meta);
}

void ApplicationImpl::draw_gui()
{
	e2e::GUI::getGUI().newFrame();

	ImGui::Begin("Controls");

	if (!boost::get<InRecovery>(&crf_state))
	{
		if (running && ImGui::Button("Pause"))
		{
			pause();
		}

		if (!running && ImGui::Button("Start"))
		{
			start();
		}

		if (!running && ImGui::Button("Recover CRF"))
		{
			start_crf_recovery();
		}
	}

	if (ImGui::Button("Exit"))
	{
		gui.w.ShouldClose(true);
	}

	ImGui::End();

	ImGui::Begin("Preview");
	ImGui::Checkbox("Undistort", &preview.undistort);
	ImGui::Checkbox("Apply Crf", &preview.apply_crf);
	if (preview.apply_crf)
	{
		ImGui::Text("Display Exposure (log):");
		ImGui::SliderFloat("Exposure", &preview.display_exposure, -20, 5, "%.0f");
	}
	ImGui::End();

	ImGui::Begin("Cameras");

	auto plot_crf = [](const crf& crf)
	{
		ImGui::PlotLines("Red (log)", [](void* data, int x)
		{
			auto d = (const float*)data;
			return std::log2(d[x]);
		}, (void*)crf.red.data(), 256, 0, "", -10, 10, ImVec2(0, 80));
		ImGui::PlotLines("Green (log)", [](void* data, int x)
		{
			auto d = (const float*)data;
			return std::log2(d[x]);
		}, (void*)crf.green.data(), 256, 0, "", -10, 10, ImVec2(0, 80));
		ImGui::PlotLines("Blue (log)", [](void* data, int x)
		{
			auto d = (const float*)data;
			return std::log2(d[x]);
		}, (void*)crf.blue.data(), 256, 0, "", -10, 10, ImVec2(0, 80));
	};

	auto draw_camera = [&plot_crf, this](json& cam_meta, e2e::uvc::stream& cam, CTL_data& ctl, auto& controller)
	{
		const std::string& n = cam_meta["name"];
		if (ImGui::TreeNode(n.c_str()))
		{
			ImGui::Text("Decoded Queue: %d", cam.get_frame_queue().size());
			CTL_data d = ctl;
			ImGui::Combo("Response Profile", &ctl.profile_index, profiles.profile_names.data(), profiles.profile_names.size());

			if (d.profile_index != ctl.profile_index)
			{
				//cam.set_profile(profiles.get_name(ctl.profile_index));
				cam_meta["profile"] = profiles.get_name(ctl.profile_index);
				reload_shaders();
			}

			if (ImGui::TreeNode("Inverse Response Function"))
			{
				plot_crf(profiles.get_response(ctl.profile_index));
				ImGui::TreePop();
			}
			const char* test[] = {
				"1 s",
				"1/2 s",
				"1/4 s",
				"1/8 s",
				"1/15 s",
				"1/30 s",
				"1/50 s",
				"1/60 s",
				"1/100 s",
				"1/250 s",
				"1/500 s",
				"1/1000 s"
			};
			ImGui::Combo("Exposure", &ctl.exposure_index, test, 12);
			if (d.exposure_index != ctl.exposure_index)
			{
				tp.push_job([&controller, &ctl, this] { controller.set_shutter_speed(cvt[ctl.exposure_index]); });
				//cam.update_exp(controller.get_exposure(d.exposure_index), d.exposure_index);
				reload_shaders();
			}
			ImGui::TreePop();
		}
	};

#if defined(E2E_FFMPEG_CAM)
	draw_camera(left_cam.get_config(), left, left_cam_ctl);
	draw_camera(right_cam.get_config(), right, right_cam_ctl);
#elif defined(E2E_UVC_CAM)
    draw_camera(left_meta, left_cam, left, left_cam_ctl);
	draw_camera(right_meta, right_cam, right, right_cam_ctl);
#endif

	ImGui::End();

	auto state = boost::get<DoneRecovery>(&crf_state);
	if (state)
	{
		ImGui::Begin("New Crf!");
		plot_crf(state->left_crf);
		plot_crf(state->right_crf);

		static char buf[128];
		ImGui::InputText("Profile Name", buf, 128);
		ImGui::SameLine();
		if (ImGui::Button("Save These"))
		{
			save_crf(state->left_crf, buf + std::string("_left"));
			save_crf(state->right_crf, buf + std::string("_right"));

			profiles.profs = find_profiles();
			profiles.update_names();


        #if defined(E2E_FFMPEG_CAM)
            auto l_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), left_cam.get_profile());
            auto r_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), right_cam.get_profile());
        #elif defined(E2E_UVC_CAM)
            auto l_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), left_meta["profile"]);
            auto r_prof_iter = std::find(profiles.profile_names.begin(), profiles.profile_names.end(), right_meta["profile"]);
        #endif

			left.profile_index = std::distance(profiles.profile_names.begin(), l_prof_iter);
			right.profile_index = std::distance(profiles.profile_names.begin(), r_prof_iter);

			crf_state = WaitingRec{};
		}

		if (ImGui::Button("Discard"))
		{
			crf_state = WaitingRec{};
		}

		ImGui::End();
	}

	static bool recompile_shaders = false;
	static int cost_choice = 0;
	static int agg_choice = 0;
	static bool detection = false;
	static bool correction = false;
	static bool median = false;
	static float threshold = 1.10;
	static float base_lum = -3.05f;
	static float max_lum = 150.0f;
	static int window_size = 7;
    static bool record = false;
    static bool show_disparity = false;



	//e2e::gui::displayStereoControl(recompile_shaders, cost_choice, agg_choice, detection, correction, median, threshold, window_size);
	e2e::gui::displayTonemapControl(base_lum, max_lum);
    e2e::gui::displayRecord(record);
    e2e::gui::displayStereoControl(recompile_shaders, cost_choice, agg_choice, detection, correction, median, threshold, window_size, show_disparity);
	merger.chooseCost(cost_choice);
	merger.chooseAggregation(agg_choice);
	merger.set_outlier_detection(detection, threshold, window_size);
	merger.set_outlier_correction(correction);
	merger.set_median_filter(median);
    merger.set_record(record);
    merger.set_display_disparity(show_disparity);

    if (gui.w.get_key_down(GLFW_KEY_A))
    {
        merger.set_color_debug((1));
    }
    else
    {
        merger.set_color_debug((0));
    }

	if (recompile_shaders)
	{
		merger.compileShaders();
	}

	merger.get_merge_shader().setUniformFVar("base", { base_lum });
	merger.get_merge_shader().setUniformFVar("maxLum", { max_lum });

	ImGui::Render();
}

void ApplicationImpl::draw_preview()
{
	left_prev_shader.setUniformFVar("prev.exposure", { preview.display_exposure });
	left_prev_shader.setUniformIVar("prev.undistort", { preview.undistort });
	left_prev_shader.setUniformIVar("prev.apply_crf", { preview.apply_crf });
    left_prev_shader.setUniformIVar("is_left", {1});

	right_prev_shader.setUniformFVar("prev.exposure", { preview.display_exposure });
	right_prev_shader.setUniformIVar("prev.undistort", { preview.undistort });
	right_prev_shader.setUniformIVar("prev.apply_crf", { preview.apply_crf });
    right_prev_shader.setUniformIVar("is_left", {0});

	gui.left_quad.draw();
	gui.right_quad.draw();
}

void ApplicationImpl::keys()
{
	auto is_pressed = [&](int key) {
		return gui.w.get_key_down(key) && prev_keys.find(key) == prev_keys.end();
	};

	for (auto it = prev_keys.begin(); it != prev_keys.end();)
	{
		if (gui.w.get_key_up(*it))
		{
			it = prev_keys.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (auto& keybind : key_handlers)
	{
		if (is_pressed(keybind.first))
		{
			keybind.second();
			prev_keys.emplace(keybind.first);
		}
	}
}

void ApplicationImpl::add_keybinding(int key, std::function<void()> &&fun) {
	key_handlers.emplace(key, std::move(fun));
}

void ApplicationImpl::start_crf_recovery() {
	if (running)
	{
		pause();
	}

	crf_state = InRecovery{};
	auto state = boost::get<InRecovery>(&crf_state);

	state->cur_exposure = 5ms;
	left_cam_ctl.set_shutter_speed(state->cur_exposure);
	right_cam_ctl.set_shutter_speed(state->cur_exposure);
	state->last_take = std::chrono::high_resolution_clock::now();

	start();
}

void ApplicationImpl::pause()
{
	left_cam.stop();
	right_cam.stop();
	left_cam.get_frame_queue().clear(); // safe since the producer threads are dead at this point
	right_cam.get_frame_queue().clear();
	running = false;
}

void ApplicationImpl::start() {
	left_cam.start();
	right_cam.start();
	running = true;
}

void ApplicationImpl::recover_routine(const FrameT &left, const FrameT &right) {
	auto state = boost::get<InRecovery>(&crf_state);
	if (state == nullptr) return;

	auto now = std::chrono::high_resolution_clock::now();
	if (now - state->last_take < std::chrono::milliseconds(1000))
	{
		return;
	}

	state->left_frames.push_back(e2e::duplicate(left));
	state->right_frames.push_back(e2e::duplicate(right));
	state->times.push_back(state->cur_exposure);
	state->cur_exposure *= 2;
    pause();

	if (state->left_frames.size() == 6)
	{
		DoneRecovery done;

		done.left_crf = e2e::app::recover_crf(state->left_frames, state->times);
		done.right_crf = e2e::app::recover_crf(state->right_frames, state->times);

        left_cam_ctl.set_shutter_speed(cvt[this->left.exposure_index]);
        right_cam_ctl.set_shutter_speed(cvt[this->right.exposure_index]);

		crf_state = std::move(done);
	}
    else
    {
        tp.push_job([this, state] {
            left_cam_ctl.set_shutter_speed(state->cur_exposure);
            right_cam_ctl.set_shutter_speed(state->cur_exposure);
            state->last_take = std::chrono::high_resolution_clock::now();
            start();
        });
    }
}

/// forwarders
Application::Application(const std::vector <std::string> &args) :
	impl(std::make_unique<ApplicationImpl>(args)) {
}

void Application::Run() {
	impl->Run();
}

Application::~Application() = default;