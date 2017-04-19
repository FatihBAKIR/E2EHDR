#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <thread>

#include <libuvc/libuvc.h>
#include <spsc/spsc_queue.h>
#include <Frame.h>
#include "e2e_uvc/camera.hpp"

int countCameras(int up_to)
{
    for (int i = 0; i<up_to; i++) {
        cv::VideoCapture temp_camera(i);
        if (!temp_camera.isOpened()) {
            temp_camera.release();
            std::cerr.flush();
            return i;
        }
        std::cerr.flush();
        temp_camera.release();
        std::cout << "has camera " << i << '\n';
    }
    return up_to;
}

using uvc_frame_ptr = std::unique_ptr<uvc_frame_t, decltype(&uvc_free_frame)>;
e2e::spsc_queue<uvc_frame_ptr> f;

void cb(uvc_frame_t* frame, void* ptr)
{
    uvc_frame_t* bgr;
    uvc_error_t ret;
    /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
    bgr = uvc_allocate_frame(frame->width*frame->height*3);
    if (!bgr) {
        printf("unable to allocate bgr frame!");
        return;
    }
    /* Do the BGR conversion */
    ret = uvc_mjpeg2rgb(frame, bgr);
    //ret = uvc_any2bgr(frame, bgr);
    if (ret) {
        uvc_perror(ret, "uvc_any2bgr");
        uvc_free_frame(bgr);
        return;
    }

    f.emplace(uvc_frame_ptr(bgr, &uvc_free_frame));
}

void test()
{
    uvc_context_t* ctx;
    uvc_device_t* dev;
    uvc_device_handle_t* devh;
    uvc_stream_ctrl_t ctrl;
    uvc_error_t res;
    /* Initialize a UVC service context. Libuvc will set up its own libusb
     * context. Replace NULL with a libusb_context pointer to run libuvc
     * from an existing libusb context. */
    res = uvc_init(&ctx, NULL);
    if (res<0) {
        uvc_perror(res, "uvc_init");
        throw res;
    }
    puts("UVC initialized");
    /* Locates the first attached UVC device, stores in dev */
    res = uvc_find_device(
            ctx, &dev,
            0x046d, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
    if (res<0) {
        uvc_perror(res, "uvc_find_device"); /* no devices found */
    }
    else {
        puts("Device found");
        /* Try to open the device: requires exclusive access */
        res = uvc_open(dev, &devh);
        if (res<0) {
            uvc_perror(res, "uvc_open"); /* unable to open device */
        }
        else {
            puts("Device opened");
            /* Print out a message containing all the information that libuvc
             * knows about the device */
            uvc_print_diag(devh, stderr);
            /* Try to negotiate a 640x480 30 fps YUYV stream profile */

            res = uvc_get_stream_ctrl_format_size(
                devh, &ctrl, /* result stored in ctrl */
                UVC_FRAME_FORMAT_ANY, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
                1280, 720, 24 /* width, height, fps */
            );

            /* Print out the result */
            uvc_print_stream_ctrl(&ctrl, stderr);
            if (res<0) {
                uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
            }
            else {
                /* Start the video stream. The library will call user function cb:
                 *   cb(frame, (void*) 12345)
                 */
                res = uvc_start_streaming(devh, &ctrl, cb, nullptr, 0);
                if (res<0) {
                    uvc_perror(res, "start_streaming"); /* unable to start stream */
                }
                else {
                    puts("Streaming...");
                    res = uvc_set_ae_mode(devh, 1); /* e.g., turn on auto exposure */
                    if (res < 0)
                    {
                        uvc_perror(res, "ae");
                    }
                    res = uvc_set_exposure_abs(devh, 50);

                    auto f_count = 0;
                    while (f_count < 200) {
                        if (f.empty()) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(20));
                            continue;
                        }

                        auto bgr = std::move(f.front());
                        f.pop();
                        auto cvImg = cvCreateImageHeader(
                                cvSize(bgr->width, bgr->height),
                                IPL_DEPTH_8U, 3);

                        cvSetData(cvImg, bgr->data, bgr->width*3);
                        cvCvtColor(cvImg, cvImg, CV_RGB2BGR);

                        cvNamedWindow("Test", CV_WINDOW_AUTOSIZE);
                        cvShowImage("Test", cvImg);
                        cvWaitKey(1);

                        cvReleaseImageHeader(&cvImg);
                        f_count++;
                    }

                    uvc_stop_streaming(devh);
                    puts("Done streaming.");
                }
            }
            /* Release our handle on the device */
            uvc_close(devh);
            puts("Device closed");
        }
        /* Release the device descriptor */
        uvc_unref_device(dev);
    }
    /* Close the UVC context. This closes and cleans up any existing device handles,
     * and it closes the libusb context if one was not provided. */
    uvc_exit(ctx);
    puts("UVC exited");
}

int main()
{
    //test();

    using namespace std::chrono_literals;
    e2e::uvc::context ctx;
    auto cams = ctx.list_cameras();

    auto cam0 = ctx.open_camera(cams[0]);
    auto cam1 = ctx.open_camera(cams[1]);

    cam0.set_wb_temp(4500);
    cam1.set_wb_temp(4000);

    cam0.set_shutter_speed(5ms);
    cam1.set_shutter_speed(5ms);

    cam0.dump();
    cam1.dump();
    cam0.set_iso(10);
    cam1.set_iso(10);

    std::cout << cam0.get_iso() <<'\n';
    std::cout << cam1.get_iso() <<'\n';

    e2e::uvc::stream s0(cam0, 1280, 720, 24);
    e2e::uvc::stream s1(cam1, 1280, 720, 24);

    s0.start();
    s1.start();

    auto& f0 = s0.get_frame_queue();
    auto& f1 = s1.get_frame_queue();

    cv::namedWindow("cam0", CV_WINDOW_AUTOSIZE);
    cv::namedWindow("cam1", CV_WINDOW_AUTOSIZE);
    auto f_count = 0;
    while (f_count < 1000) {
        std::cerr << "lens: " << f0.size() << ", " << f1.size() << '\n';
        if (f0.empty() || f1.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        auto bgr0 = std::move(f0.front());
        f0.pop();

        auto bgr1 = std::move(f1.front());
        f1.pop();

        cv::Mat frame0(cv::Size(bgr0.width(), bgr0.height()), CV_8UC3, bgr0.buffer().data(), cv::Mat::AUTO_STEP);
        cv::Mat frame1(cv::Size(bgr1.width(), bgr1.height()), CV_8UC3, bgr1.buffer().data(), cv::Mat::AUTO_STEP);

        cv::cvtColor(frame0, frame0, CV_RGB2BGR);
        cv::cvtColor(frame1, frame1, CV_RGB2BGR);

        cv::imshow("cam0", frame0);
        cv::imshow("cam1", frame1);
        cv::waitKey(1);

        f_count++;
    }

    //auto x =  countCameras(2) ;
    //std::cout << x << std::endl;
/*
    cv::VideoCapture cam(0);

    cam.set(CV_CAP_PROP_FOURCC, CV_FOURCC('H', '2', '6', '4'));
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    cam.set(CV_CAP_PROP_FPS, 24);
    std::cout << cam.set(CV_CAP_PROP_AUTO_EXPOSURE, 0 ) << '\n';
    std::cout << cam.set(CV_CAP_PROP_EXPOSURE, -5.f) << '\n';


    std::cout << cam.get(CV_CAP_PROP_EXPOSURE) << '\n';


    cv::Mat data;
    cam >> data;

    cv::imwrite("test.png", data);*/

    return 0;
}