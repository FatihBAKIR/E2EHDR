//
// Created by Mehmet Fatih BAKIR on 11/04/2017.
//

#pragma once

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <Frame.h>
#include <spsc/spsc_queue.h>

namespace e2e
{

namespace bi = boost::interprocess;
struct per_frame
{
	e2e::LDRFrame tmo;
	e2e::LDRFrame res;
};

class shared_frames_queue
{
	struct shm_data
	{
		e2e::spsc_queue<per_frame> q;
	};

	bi::shared_memory_object smo;
	bi::mapped_region region;
    bi::message_queue mq;

	shm_data* data;
	short width;
	short height;

	void setup_shm() // only call from camera
	{
		smo.truncate(sizeof(shm_data));
		data = new (region.get_address()) shm_data;
	}

	void setup_player()
	{
		data = reinterpret_cast<shm_data*>(region.get_address());
	}

public:

	shared_frames_queue(bool is_camera, int w, int h) :
			mq(bi::open_or_create, "hdr_stream_mq", 100, w * h * 3 * sizeof(uint8_t) * 2)
	{
		if (is_camera)
		{
			setup_shm();
		}
		else
		{
			setup_player();
		}
		width = w;
		height = h;
	}

    void push(const e2e::LDRFrame& tmo, const e2e::LDRFrame& resid)
	{
		mq.send(tmo.buffer().data(), tmo.buffer().size(), 0);
		mq.send(resid.buffer().data(), resid.buffer().size(), 1);
	}


	per_frame receive()
	{
		auto tmo_data = std::make_unique<uint8_t[]>(width * height * 3);
		auto resid_data = std::make_unique<uint8_t[]>(width * height * 3);

		unsigned pr;
		bi::message_queue::size_type sz;

		mq.receive(tmo_data.get(), width * height * 3 * sizeof(uint8_t), sz, pr);
		mq.receive(resid_data.get(), width * height * 3 * sizeof(uint8_t), sz, pr);

		return { {std::move(tmo_data), width, height},{std::move(resid_data), width, height},};
	}
};

}
