//
// Created by Mehmet Fatih BAKIR on 19/12/2016.
//

#include "camera_struct.h"

template <class T, class Fun>
auto apply(T&& elem, Fun f)
{
    return f(std::forward<T>(elem));
}

template <class BeginT, class Fun, class... FunTs>
auto apply(BeginT&& elem, Fun f, FunTs... fs)
{
    return apply(f(std::forward<BeginT>(elem)), fs...);
};

template <class InputQueueT, class OutputQueueT, class... Rest>
auto pipeline(InputQueueT& in, OutputQueueT& out, Rest... funs)
{
    return boost::thread(
            [&, funs...]
            {
                while (!boost::this_thread::interruption_requested())
                {
                    if (in.empty()) continue;

                    auto elem = std::move(in.front());
                    in.pop();

                    auto res = apply(elem, funs...);
                    if (res)
                    {
                        out.emplace(std::move(res.get()));
                    }
                }
            });
};

void camera_struct::start() {
    pull_thread = boost::thread([this]{ camera.start_capture(); });
    auto handler = [this](auto&& p) { return handle_packet(std::forward<decltype(p)>(p)); };
    decode_thread = pipeline(camera.packet_queue, frame_queue, handler);
}

void camera_struct::stop() {
    pull_thread.interrupt();
    decode_thread.interrupt();

    pull_thread.join();
    decode_thread.join();
}

void camera_struct::recycle(camera_struct::FrameT &&f) {
    decoder.return_buffer(f.u_ptr());
}

float camera_struct::get_exposure() const {
    return config["exposure"];
}

/*const crf& camera_struct::get_response() const {
    if (!response)
    {
        response = load_crf(config["crf"]);
    }

    return response.get();
}*/

undistort camera_struct::get_undistort() const {
    return ::get_undistort(config);
}

camera_struct::~camera_struct() {
    stop();
}

camera_struct::camera_struct(const nlohmann::json &config) :
        config(config),
        camera((const std::string&)config["rtsp_url"]),
        decoder(camera.codec_ctx_),
        name ((const std::string&)config["name"]),
        ip ((const std::string&)config["ip"])
{}

boost::optional<camera_struct::FrameT> camera_struct::handle_packet(e2e::ff::Camera::FrameData frame_data) {
    if(frame_data.packet->stream_index == camera.video_stream_index_)
    {
        auto ret = decoder.decode_one(frame_data.packet);
        av_packet_unref(frame_data.packet);

        if (ret)
        {
            // int w, int h, e2e::ff::Decoder::data_ptr data
            auto f = (FrameT(std::move(ret), camera.codec_ctx_->width, camera.codec_ctx_->height));
            f.set_time(frame_data.timestamp);
            return std::move(f);
        }
    }

    std::cout << "nope\n";
    return boost::none; 
}

const std::string& camera_struct::get_name() const {
    return name;
}

const std::string &camera_struct::get_ip() const {
    return ip;
}

int camera_struct::get_exp_code() const {
    return config["exp_code"];
}

void camera_struct::update_exp(float ex, int code) {
    config["exposure"] = ex;
    config["exp_code"] = code;
}

void camera_struct::set_profile(const std::string& prof)
{
    config["profile"] = prof;
}

std::string camera_struct::get_profile() const
{
	std::cout << config << '\n';
    return config["profile"];
}

/*void camera_struct::set_response(const crf& crf)
{
    response = crf;
}*/
