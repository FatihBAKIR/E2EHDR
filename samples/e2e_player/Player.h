//
// Created by Göksu Güvendiren on 28/03/2017.
//

#ifndef E2E_PLAYER_PLAYER_H
#define E2E_PLAYER_PLAYER_H

#include <queue>
#include <opencv2/opencv.hpp>
#include <Frame.h>
#include <spsc/spsc_queue.h>

#include <Window.h>

class Player
{
    e2e::spsc_queue<e2e::HDRFrame, e2e::constant_storage<e2e::HDRFrame, 128>> frames;


public:
    Player();

    auto& Frames() { return frames; }
};


#endif //E2E_PLAYER_PLAYER_H
