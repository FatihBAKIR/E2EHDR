//
// Created by Mehmet Fatih BAKIR on 20/12/2016.
//

#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

namespace e2e
{
    class thread_pool
    {
        boost::asio::io_service ioService;
        boost::thread_group threadpool;
        boost::asio::io_service::work work;
    public:
        thread_pool(int t_count) :
                work(ioService)
        {
            for (int i = 0; i < t_count; ++i)
            {
                threadpool.create_thread(
                        boost::bind(&boost::asio::io_service::run, &ioService)
                );
            }
        }

        void stop()
        {
            ioService.stop();
            threadpool.join_all();
        }

        void push_job(const std::function<void(void)>& job)
        {
            ioService.post(job);
        }

        ~thread_pool()
        {
            stop();
        }
    };
}