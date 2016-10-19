#include <iostream>
#include <gphoto2/gphoto2.h>
#include <chrono>
#include <thread>
#include <OpenGL/gl.h>

static void
ctx_error_func(GPContext *context, const char *str, void *data) {
    fprintf(stderr, "\n*** Contexterror ***              \n%s\n", str);
    fflush(stderr);
}

static void
ctx_status_func(GPContext *context, const char *str, void *data) {
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

int
sample_autodetect(CameraList *list, GPContext *context) {
    gp_list_reset(list);
    return gp_camera_autodetect(list, context);
}
static GPPortInfoList		*portinfolist = NULL;
static CameraAbilitiesList	*abilities = NULL;

int
sample_open_camera (Camera ** camera, const char *model, const char *port, GPContext *context) {
    int		ret, m, p;
    CameraAbilities	a;
    GPPortInfo	pi;

    ret = gp_camera_new (camera);
    if (ret < GP_OK) return ret;

    if (!abilities) {
        /* Load all the camera drivers we have... */
        ret = gp_abilities_list_new (&abilities);
        if (ret < GP_OK) return ret;
        ret = gp_abilities_list_load (abilities, context);
        if (ret < GP_OK) return ret;
    }

    /* First lookup the model / driver */
    m = gp_abilities_list_lookup_model (abilities, model);
    if (m < GP_OK) return ret;
    ret = gp_abilities_list_get_abilities (abilities, m, &a);
    if (ret < GP_OK) return ret;
    ret = gp_camera_set_abilities (*camera, a);
    if (ret < GP_OK) return ret;

    if (!portinfolist) {
        /* Load all the port drivers we have... */
        ret = gp_port_info_list_new (&portinfolist);
        if (ret < GP_OK) return ret;
        ret = gp_port_info_list_load (portinfolist);
        if (ret < 0) return ret;
        ret = gp_port_info_list_count (portinfolist);
        if (ret < 0) return ret;
    }

    /* Then associate the camera with the specified port */
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

static void
capture_to_memory(Camera *camera, GPContext *context, const char **ptr, unsigned long int *size) {
    int retval;
    CameraFile *file;
    CameraFilePath camera_file_path;

    printf("Capturing.\n");

    /* NOP: This gets overridden in the library to /capt0000.jpg */
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

    //gp_file_free(file);
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    Camera *c;
    gp_camera_new(&c);

    GPContext *context = sample_create_context();

    CameraList *cams;
    gp_list_new(&cams);
    sample_autodetect(cams, context);
    for (int i = 0; i < gp_list_count(cams); i++) {
        const char *name, *value;
        gp_list_get_name(cams, i, &name);
        gp_list_get_value(cams, i, &value);
        std::cout << name << ' ' << value << '\n';
    }
    gp_list_free(cams);

    sample_open_camera(&c, "Canon EOS 600D", "usb:020,012", context);

    const char* data;
    unsigned long size;
    capture_to_memory(c, context, &data, &size);

    std::cout << size << data[0] << '\n';

    GLuint tex;
    glGenTextures(1, &tex);

    return 0;
}