#pragma once
#include "pch.h"

class DynamicThreadPool : private boost::noncopyable_::base_token
{
public:

    DynamicThreadPool(asio::io_context& io_context, std::vector<boost::thread>& threads, unsigned int max_threads, long threshold_seconds, long min_seconds, long periodic_seconds)
        : _io_context(io_context), _timer(_io_context), _threads(threads), _max_threads(max_threads), _threshold_seconds(threshold_seconds), _min_seconds(min_seconds), _periodic_seconds(periodic_seconds)
    {
        schedule_check();
    }

private:
    void schedule_check();
    void on_check(const boost::system::error_code& error);

    boost::asio::io_context& _io_context;
    boost::asio::deadline_timer _timer;

    std::vector<boost::thread>& _threads;
    unsigned int _max_threads;
    long _threshold_seconds;
    long _min_seconds;
    long _periodic_seconds;
};
