//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//

#include <iostream>
#include <gphoto2/gphoto2.h>
#include "gphoto_wrapper.h"
#include "jpeglib.h"

void ctx_error_func(GPContext *context, const char *str, void *data) {
    fprintf(stderr, "\n*** Contexterror ***              \n%s\n", str);
    fflush(stderr);
}

void ctx_status_func(GPContext *context, const char *str, void *data) {
    fprintf(stderr, "%s\n", str);
    fflush(stderr);
}

GPContext *sample_create_context() {
    GPContext *context;
    context = gp_context_new();
    gp_context_set_error_func(context, ctx_error_func, NULL);
    gp_context_set_status_func(context, ctx_status_func, NULL);
    return context;
}

GPPortInfoList		*portinfolist = nullptr;
CameraAbilitiesList	*abilities = nullptr;

int sample_open_camera (Camera ** camera, const char *model, const char *port, GPContext *context) {
    int		ret, m, p;
    CameraAbilities	a;
    GPPortInfo	pi;

    ret = gp_camera_new (camera);
    if (ret < GP_OK) return ret;

    if (!abilities) {
        ret = gp_abilities_list_new (&abilities);
        if (ret < GP_OK) return ret;
        ret = gp_abilities_list_load (abilities, context);
        if (ret < GP_OK) return ret;
    }

    m = gp_abilities_list_lookup_model (abilities, model);
    if (m < GP_OK) return ret;
    ret = gp_abilities_list_get_abilities (abilities, m, &a);
    if (ret < GP_OK) return ret;
    ret = gp_camera_set_abilities (*camera, a);
    if (ret < GP_OK) return ret;

    if (!portinfolist) {
        ret = gp_port_info_list_new (&portinfolist);
        if (ret < GP_OK) return ret;
        ret = gp_port_info_list_load (portinfolist);
        if (ret < 0) return ret;
        ret = gp_port_info_list_count (portinfolist);
        if (ret < 0) return ret;
    }

    p = gp_port_info_list_lookup_path (portinfolist, port);
    switch (p) {
        case GP_ERROR_UNKNOWN_PORT:
            fprintf (stderr, "The port you specified "
                             "('%s') can not be found. Please "
                             "specify one of the ports found by "
                             "'gphoto2 --list-ports' and make "
                             "sure the spelling is correct "
                             "(i.e. with prefix 'serial:' or 'usb:').",
                     port);
            break;
        default:
            break;
    }
    if (p < GP_OK) return p;

    ret = gp_port_info_list_get_info (portinfolist, p, &pi);
    if (ret < GP_OK) return ret;
    ret = gp_camera_set_port_info (*camera, pi);
    if (ret < GP_OK) return ret;
    return GP_OK;
}

void capture_to_memory(Camera *camera, GPContext *context, const char **ptr, unsigned long int *size) {
    int retval;
    CameraFile *file;
    CameraFilePath camera_file_path;

    printf("Capturing.\n");

    strcpy(camera_file_path.folder, "/");
    strcpy(camera_file_path.name, "foo.jpg");
    retval = gp_file_new(&file);
    gp_camera_capture_preview(camera, file, context);


    printf("  Retval: %d == %d\n", retval, GP_OK);

    retval = gp_file_get_data_and_size (file, ptr, size);
    const char* mime;
    gp_file_get_mime_type(file, &mime);

    printf("mime: %s\n", mime);

    printf("  Retval: %d\n", retval);
    gp_file_unref(file);
}

namespace e2e
{
namespace gp
{

    Camera::Camera(const CameraInfo &info, GPhoto &gp) :
            gp_(gp)
    {
        gp_camera_new(&ptr_);
        sample_open_camera(&ptr_, info.first.c_str(), info.second.c_str(), gp_.ctx_);
    }

    Camera::~Camera()
    {
        gp_camera_free(ptr_);
    }

    LDRFrame Camera::LiveviewFrame()
    {
        int retval;
        CameraFile *file;

        retval = gp_file_new(&file);

        if (retval != GP_OK)
        {
            throw capture_error("gp_file_new failed!");
        }

        retval = gp_camera_capture_preview(ptr_, file, gp_.ctx_);

        if (retval != GP_OK)
        {
            throw capture_error("Preview capture failed");
        }

        const char* buffer;
        unsigned long size;

        retval = gp_file_get_data_and_size (file, &buffer, &size);

        if (retval != GP_OK)
        {
            throw capture_error("File get data failed");
        }

        const char* mime;
        retval = gp_file_get_mime_type(file, &mime);

        if (retval != GP_OK)
        {
            throw capture_error("Mime type failed");
        }

        printf("mime: %s\n", mime);

        e2e::decode_jpeg(gsl::span<const gsl::byte>{reinterpret_cast<const gsl::byte *>(buffer), static_cast<long>(size)});

        gp_file_free(file);
    }

    GPhoto::GPhoto() : ctx_(sample_create_context()) {}

    std::vector<CameraInfo> GPhoto::ListCameras()
    {
        CameraList *cams;
        gp_list_new(&cams);

        gp_list_reset(cams);
        gp_camera_autodetect(cams, ctx_);

        std::vector<CameraInfo> cameras;

        for (int i = 0; i < gp_list_count(cams); i++) {
            const char *name, *value;
            gp_list_get_name(cams, i, &name);
            gp_list_get_value(cams, i, &value);
            cameras.emplace_back(name, value);
        }

        gp_list_free(cams);

        return cameras;
    };

}
}