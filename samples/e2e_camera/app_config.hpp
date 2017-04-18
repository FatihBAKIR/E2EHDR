//
// Created by Mehmet Fatih BAKIR on 19/04/2017.
//

#pragma once

#include <Frame.h>

namespace e2e
{
namespace app
{
#if defined(E2E_FFMPEG_CAM)
using FrameT = camera_struct::FrameT;
#elif defined(E2E_UVC_CAM)
using FrameT = e2e::LDRFrame;
#endif
}
}
