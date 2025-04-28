#include "DynamicThreadPool.h"

void DynamicThreadPool::schedule_check()
{
    // Thread pool is already at max size.
    if (_max_threads <= _threads.size())
    {
        std::cout << "Thread pool has reached its max" << std::endl;
    }

    std::cout << "Will check if pool needs to increase in "
        << _periodic_seconds << " seconds." << std::endl;

    _timer.expires_from_now(boost::posix_time::seconds(_periodic_seconds));
    _timer.async_wait(
        boost::bind(&DynamicThreadPool::on_check, this,
            boost::asio::placeholders::error));
}

void DynamicThreadPool::on_check(const boost::system::error_code& error)
{
    // On error, return early.
    if (error)
    {
        fail(error, "check threadpool");
        return;
    }

    // Check how long this job was waiting in the service queue.  This
    // returns the expiration time relative to now.  Thus, if it expired
    // 7 seconds ago, then the delta time is -7 seconds.
    boost::posix_time::time_duration delta = _timer.expires_from_now();
    long wait_in_seconds = -delta.seconds();

    // If the time delta is greater than the threshold, then the job
    // remained in the service queue for too long, so increase the
    // thread pool.
    //std::cout << "Job job sat in queue for "
    //    << wait_in_seconds << " seconds." << std::endl;

    if (_threshold_seconds < wait_in_seconds)
    {
        std::cout << "Increasing thread pool." << std::endl;
        _threads.push_back(boost::thread(boost::bind(&boost::asio::io_context::run,
            &_io_context)));
    }
    else if (_min_seconds > wait_in_seconds)
    {
        std::cout << "Decrease thread pool." << std::endl;
        _threads.back().join();
        _threads.pop_back();
    }

    schedule_check();
}
