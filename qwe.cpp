#include "asd.h"
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

boost::asio::io_context ctx;

AsyncMutx mutex;
boost::asio::awaitable<int> qwe() {
    auto lock=co_await mutex.lock();
    boost::asio::deadline_timer timer{ctx, boost::posix_time::seconds{1}};
    co_await timer.async_wait(boost::asio::use_awaitable);
    printf("qwe\n");
    co_return 5;
}

boost::asio::awaitable<void> asd() {
    /*
    {
        boost::asio::deadline_timer timer{ctx, boost::posix_time::seconds{2}};
        auto ret = parrelSpwan(qwe(), ctx);
        co_await timer.async_wait(boost::asio::use_awaitable);
        printf("asd\n");
        auto a = co_await std::move(ret);
        printf("result:%d\n", a);
    }
    {
        boost::asio::deadline_timer timer{ctx, boost::posix_time::seconds{2}};
        auto ret=qwe();
        co_await timer.async_wait(boost::asio::use_awaitable);
        printf("asd\n");
        auto a=co_await std::move(ret);
        printf("result:%d\n",a);
    }*/
    {
        ASD a;
        boost::asio::co_spawn(ctx, a.wrapper(qwe), boost::asio::detached);
        boost::asio::co_spawn(ctx, a.wrapper(qwe), boost::asio::detached);
        boost::asio::co_spawn(ctx, a.wrapper(qwe), boost::asio::detached);
        boost::asio::co_spawn(ctx, a.wrapper(qwe), boost::asio::detached);
        boost::asio::deadline_timer timer{ctx, boost::posix_time::seconds{2}};
        co_await timer.async_wait(boost::asio::use_awaitable);
        printf("after all");
        co_await a.waitAll();
        printf("real after all");
    }
    co_return;
}

int main() {
    auto fu = boost::asio::co_spawn(ctx, asd(), boost::asio::use_future);;
    std::thread thr;
    {
        auto guard = boost::asio::make_work_guard(ctx);
        std::thread{[]() {
            ctx.run();
        }}.swap(thr);
        fu.get();
    }
    thr.join();
}