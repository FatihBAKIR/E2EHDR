#include <iostream>
#include "X264Encoder.h"

X264Encoder::X264Encoder()
        :in_width(0), in_height(0), in_pixel_format(AV_PIX_FMT_NONE), out_width(0), out_height(0),
         out_pixel_format(AV_PIX_FMT_NONE), fps(25), fp(nullptr), encoder(nullptr), sws(nullptr), num_nals(0), pts(0)
{
    memset((char*) &pic_raw, 0, sizeof(pic_raw));
}

X264Encoder::~X264Encoder()
{
    if (sws) {
        close();
    }
}

bool X264Encoder::open(std::string filename)
{
    validateSettings(); // throws runtime_error if anything is wrong

    int r = 0;
    int nheader = 0;
    int header_size = 0;

    // @todo add validate which checks if all params are set (in/out width/height, fps,etc..);
    if (encoder) {
        throw std::runtime_error("Already opened. first call close()");
    }

    if (out_pixel_format!=AV_PIX_FMT_YUV420P) {
        throw std::runtime_error("At this moment the output format must be AV_PIX_FMT_YUV420P");
    }

    sws = sws_getContext(in_width, in_height, in_pixel_format,
            out_width, out_height, out_pixel_format,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws) {
        throw std::runtime_error("Cannot create SWS context");
    }

    fp = fopen(filename.c_str(), "w+b");
    if (!fp) {
        //FIXME : file descriptor leak : close()
        throw std::runtime_error("Cannot open the h264 destination file");
    }

    x264_picture_alloc(&pic_in, X264_CSP_I420, out_width, out_height);

    setParams();

    // create the encoder using our params
    encoder = x264_encoder_open(&params);
    if (!encoder) {
        throw std::runtime_error("Cannot open the encoder");
    }

    // write headers
    r = x264_encoder_headers(encoder, &nals, &nheader);
    if (r<0) {
        throw std::runtime_error("x264_encoder_headers() failed");
    }

    header_size = nals[0].i_payload+nals[1].i_payload+nals[2].i_payload;
    if (!fwrite(nals[0].p_payload, header_size, 1, fp)) {
        throw std::runtime_error("Cannot write headers");
    }

    pts = 0;
    return true;
}

bool X264Encoder::encode(char* pixels, std::function<void(uint8_t* data, int len)> handler)
{
    if (!sws) {
        throw std::runtime_error("Not initialized, so cannot encode");
    }

    // copy the pixels into our "raw input" container.
    int bytes_filled = avpicture_fill(&pic_raw, (uint8_t*) pixels, in_pixel_format, in_width, in_height);
    if (!bytes_filled) {
        throw std::runtime_error("Cannot fill the raw input buffer");
    }

    // convert to I420 for x264
    int h = sws_scale(sws, pic_raw.data, pic_raw.linesize, 0,
            in_height, pic_in.img.plane, pic_in.img.i_stride);

    if (h!=out_height) {
        throw std::runtime_error("scale failed: "+std::to_string(h));
        return false;
    }

    // and encode and store into pic_out
    pic_in.i_pts = pts;

    auto wtv = pic_in.extra_sei;

    x264_sei_t sei;
    sei.num_payloads = 1;
    sei.payloads = new x264_sei_payload_t[1];
    sei.sei_free = [](void* data) {
        delete (x264_sei_payload_t*) data;
    };
    sei.payloads[0].payload = new uint8_t[666];
    uint32_t data = 0xDEADBEEF;
    uint32_t * p = (uint32_t *) sei.payloads[0].payload, i;

    for(i = 0; i < 666 / sizeof(* p); ++i) {
        p[i] = data;
    }

    sei.payloads[0].payload_size = 666;
    sei.payloads[0].payload_type = 5;
    pic_in.extra_sei = sei;


    int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, &pic_in, &pic_out);
    if (frame_size) {
        if (!fwrite(nals[0].p_payload, frame_size, 1, fp)) {
            throw std::runtime_error("Error while trying to write nal");
        }

        handler(nals[0].p_payload, frame_size);
    }
    ++pts;

    return true;
}

bool X264Encoder::close()
{
    if (encoder) {
        while (x264_encoder_delayed_frames(encoder)) {
            auto frame_size = x264_encoder_encode(encoder, &nals, &num_nals, nullptr, &pic_out);
            if (!fwrite(nals[0].p_payload, frame_size, 1, fp)) {
                throw std::runtime_error("Error while trying to write nal");
            }
        }

        x264_picture_clean(&pic_in);
        memset((char*) &pic_in, 0, sizeof(pic_in));
        memset((char*) &pic_out, 0, sizeof(pic_out));

        x264_encoder_close(encoder);
        encoder = nullptr;
    }

    if (sws) {
        sws_freeContext(sws);
        sws = nullptr;
    }

    memset((char*) &pic_raw, 0, sizeof(pic_raw));

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }
    return true;
}

void X264Encoder::setParams()
{
    x264_param_default_preset(&params, "ultrafast", "zerolatency");
    params.i_threads = 1;
    params.i_width = out_width;
    params.i_height = out_height;
    params.i_fps_num = fps;
    params.i_fps_den = 1;
}

bool X264Encoder::validateSettings()
{
    if (!in_width) {
        throw std::runtime_error("No in_width set");
    }
    if (!in_height) {
        throw std::runtime_error("No in_height set");
    }
    if (!out_width) {
        throw std::runtime_error("No out_width set");
    }
    if (!out_height) {
        throw std::runtime_error("No out_height set");
    }
    if (in_pixel_format==AV_PIX_FMT_NONE) {
        throw std::runtime_error("No in_pixel_format set");
    }
    if (out_pixel_format==AV_PIX_FMT_NONE) {
        throw std::runtime_error("No out_pixel_format set");
    }
    return true;
}
