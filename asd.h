#ifndef CORO_UTILS_ASD_H
#define CORO_UTILS_ASD_H

#include "boost/asio/awaitable.hpp"
#include "boost/asio/async_result.hpp"
#include "boost/asio/use_awaitable.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/detached.hpp"
class ASD {
    std::atomic<uint64_t> seq{0};
    using handler_type = boost::asio::async_result<boost::asio::use_awaitable_t<>, void()>::handler_type;

    union asd {
        char buf[0];
        handler_type handler;

        asd() {}

        ~asd() {}
    } value;

public:
    template<class T>
    auto wrapper(T &&obj) {
        return [this, obj = std::move(obj)]()mutable -> boost::asio::awaitable<void> {
            seq++;
            co_await std::move(obj)();
            auto ret = seq--;
            if (ret == 0) {
                value.handler();
                value.handler.~handler_type();
            }
        };
    }

    boost::asio::awaitable<void> waitAll() {
        return boost::asio::async_initiate<decltype(boost::asio::use_awaitable), void()>([this](auto &&a) {
            auto index = seq--;
            if (index != 0) {
                new(&value.handler)handler_type{std::move(a)};
            } else {
                a();
            }
        }, boost::asio::use_awaitable);
    }
};

#include <memory>
template <class T,class EX>
boost::asio::awaitable<T> parrelSpwan(boost::asio::awaitable<T> &&obj,EX &ex){
    using handler_type = typename boost::asio::async_result<boost::asio::use_awaitable_t<>, void(T)>::handler_type;
    class AwaitInfo{
    public:
        bool is_trigger;
        union ret_handle_t{
            T value;
            handler_type handler;
            ret_handle_t(){}
            ~ret_handle_t(){}
        }ret_handle;
    };
    auto ptr=std::make_shared<AwaitInfo>();

    boost::asio::co_spawn(ex,[obj=std::move(obj),ptr]()mutable->boost::asio::awaitable<void>{
        auto value=co_await std::move(obj);
        if(ptr->is_trigger){
            ptr->ret_handle.handler(std::move(value));
            ptr->ret_handle.handler.~handler_type();
        }else{
            new (&ptr->ret_handle.value)T{std::move(value)};
        }
        ptr->is_trigger=true;
        co_return;
    },boost::asio::detached);
    return boost::asio::async_initiate<decltype(boost::asio::use_awaitable), void(T)>([ptr](auto &&a) {
        if(ptr->is_trigger){
            a(std::move(ptr->ret_handle.value));
            ptr->ret_handle.value.~T();
        }else{
            new (&ptr->ret_handle.handler)handler_type{std::move(a)};
            ptr->is_trigger=true;
        }
    }, boost::asio::use_awaitable);
}
#endif //CORO_UTILS_ASD_H
