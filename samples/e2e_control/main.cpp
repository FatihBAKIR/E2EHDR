#include "curl_easy.h"
#include "curl_exception.h"

using curl::curl_easy;
using curl::curl_form;
using curl::curl_pair;

using curl::curl_easy_exception;
using curl::curlcpp_traceback;

int main(int argc, const char **argv)
{
    std::vector<std::string> headers;
    headers.push_back("Cookie: snc_cview_auto_play=");
    headers.push_back("Origin: http://192.168.200.20");
    headers.push_back("Accept-Encoding: gzip, deflate");
    headers.push_back("Accept-Language: en-US,en;q=0.8,tr;q=0.6");
    headers.push_back("Upgrade-Insecure-Requests: 1");
    headers.push_back("Authorization: Basic YWRtaW46YWRtaW4=");
    headers.push_back("Content-Type: application/x-www-form-urlencoded");
    headers.push_back("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    headers.push_back("User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.95 Safari/537.36");
    headers.push_back("Cache-Control: max-age=0");
    headers.push_back("Referer: http://192.168.200.20/en/l4/camera/picture.html");
    headers.push_back("Connection: keep-alive");
    headers.push_back("DNT: 1");

    curl_slist list[13];
    for (auto i = 0; i < 12; i++){
        list[i].data = &headers[i][0];
        list[i].next = &list[i+1];
    }

    std::vector<char> cstr(headers[13].c_str(), headers[13].c_str() + headers[13].size() + 1);
    list[13].data = cstr.data();
    list[13].next = nullptr;


    curl_form form;
    // Forms creation
    curl_pair<CURLformoption,std::string> BLComp_form(CURLFORM_COPYNAME,"BLComp");
    curl_pair<CURLformoption,std::string> BLComp_cont(CURLFORM_COPYCONTENTS,"off");
    curl_pair<CURLformoption,std::string> ExpComp_form(CURLFORM_COPYNAME,"ExpComp");
    curl_pair<CURLformoption,std::string> ExpComp_cont(CURLFORM_COPYCONTENTS,"6");
    curl_pair<CURLformoption,std::string> Agc_form(CURLFORM_COPYNAME,"Agc");
    curl_pair<CURLformoption,std::string> Agc_cont(CURLFORM_COPYCONTENTS,"off");
    curl_pair<CURLformoption,std::string> AutoSlowShutter_form(CURLFORM_COPYNAME,"AutoSlowShutter");
    curl_pair<CURLformoption,std::string> AutoSlowShutter_cont(CURLFORM_COPYCONTENTS,"on");
    curl_pair<CURLformoption,std::string> AutoSlowShutterMinSpeed_form(CURLFORM_COPYNAME,"AutoSlowShutterMinSpeed");
    curl_pair<CURLformoption,std::string> AutoSlowShutterMinSpeed_cont(CURLFORM_COPYCONTENTS,"7");
    curl_pair<CURLformoption,std::string> AutoShutter_form(CURLFORM_COPYNAME,"AutoShutter");
    curl_pair<CURLformoption,std::string> AutoShutter_cont(CURLFORM_COPYCONTENTS,"on");
    curl_pair<CURLformoption,std::string> AutoShutterMaxSpeed_form(CURLFORM_COPYNAME,"AutoShutterMaxSpeed");
    curl_pair<CURLformoption,std::string> AutoShutterMaxSpeed_cont(CURLFORM_COPYCONTENTS,"7");
    curl_pair<CURLformoption,std::string> WBMode_form(CURLFORM_COPYNAME,"WBMode");
    curl_pair<CURLformoption,std::string> WBMode_cont(CURLFORM_COPYCONTENTS,"atw");
    curl_pair<CURLformoption,std::string> VideoNoiseReduction_form(CURLFORM_COPYNAME,"VideoNoiseReduction");
    curl_pair<CURLformoption,std::string> VideoNoiseReduction_cont(CURLFORM_COPYCONTENTS,"on");
    curl_pair<CURLformoption,std::string> Gamma_form(CURLFORM_COPYNAME,"Gamma");
    curl_pair<CURLformoption,std::string> Gamma_cont(CURLFORM_COPYCONTENTS,"0");
    curl_pair<CURLformoption,std::string> Brightness_form(CURLFORM_COPYNAME,"Brightness");
    curl_pair<CURLformoption,std::string> Brightness_cont(CURLFORM_COPYCONTENTS,"5");
    curl_pair<CURLformoption,std::string> Saturation_form(CURLFORM_COPYNAME,"Saturation");
    curl_pair<CURLformoption,std::string> Saturation_cont(CURLFORM_COPYCONTENTS,"3");
    curl_pair<CURLformoption,std::string> Sharpness_form(CURLFORM_COPYNAME,"Sharpness");
    curl_pair<CURLformoption,std::string> Sharpness_cont(CURLFORM_COPYCONTENTS,"3");
    curl_pair<CURLformoption,std::string> Contrast_form(CURLFORM_COPYNAME,"Contrast");
    curl_pair<CURLformoption,std::string> Contrast_cont(CURLFORM_COPYCONTENTS,"3");
    curl_pair<CURLformoption,std::string> reload_form(CURLFORM_COPYNAME,"reload");
    curl_pair<CURLformoption,std::string> reload_cont(CURLFORM_COPYCONTENTS,"referer");


    try {
        form.add(BLComp_form, BLComp_cont);
        form.add(ExpComp_form, ExpComp_cont);
        form.add(Agc_form, Agc_cont);
        form.add(AutoSlowShutter_form, AutoSlowShutter_cont);
        form.add(AutoSlowShutterMinSpeed_form, AutoSlowShutterMinSpeed_cont);
        form.add(AutoShutter_form, AutoShutter_cont);
        form.add(AutoShutterMaxSpeed_form, AutoShutterMaxSpeed_cont);
        form.add(WBMode_form, WBMode_cont);
        form.add(VideoNoiseReduction_form, VideoNoiseReduction_cont);
        form.add(Gamma_form, Gamma_cont);
        form.add(Brightness_form, Brightness_cont);
        form.add(Saturation_form, Saturation_cont);
        form.add(Sharpness_form, Sharpness_cont);
        form.add(Contrast_form, Contrast_cont);
        form.add(reload_form, reload_cont);


        curl_easy easy;
        // Add some option to the curl_easy object.
        easy.add<CURLOPT_URL>("http://192.168.200.20/command/camera.cgi");
        easy.add<CURLOPT_FOLLOWLOCATION>(1L);
        easy.add<CURLOPT_HTTPPOST>(form.get());

        easy.add<CURLOPT_HTTPHEADER>(list);

        // Execute the request.
        easy.perform();
    }
    catch (curl_easy_exception error) {
        // If you want to get the entire error stack we can do:
        curlcpp_traceback errors = error.get_traceback();
        // Otherwise we could print the stack like this:
        error.print_traceback();
        // Note that the printing the stack will erase it
    }
    return 0;
}