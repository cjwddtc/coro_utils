#include "asd.h"
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
boost::asio::io_context ctx;
boost::asio::awaitable<int> qwe(){
    boost::asio::deadline_timer timer{ctx,boost::posix_time::seconds{1}};
    co_await timer.async_wait(boost::asio::use_awaitable);
    printf("qwe\n");
    co_return 5;
}
boost::asio::awaitable<void> asd(){
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
    }
    co_return;
}
int main(){
    auto fu=boost::asio::co_spawn(ctx,asd(),boost::asio::use_future);
    std::thread thr([](){
        ctx.run();
    });
    fu.get();
}