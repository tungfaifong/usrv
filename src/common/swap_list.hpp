// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_SWAP_LIST_HPP
#define USRV_SWAP_LIST_HPP

#include <list>
#include <mutex>
#include <memory>
#include <functional>

namespace usrv
{
    template<typename T>
    class SwapList
    {
    public:
        SwapList() : list_(std::make_unique<std::list<T>>()),
            swap_list_(std::make_unique<std::list<T>>())
        {
        }

    public:
        void EmplaceBack(T&& item)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            list_->emplace_back(std::forward<T>(item));
        }

        void ForEach(std::function<void(const T &)> func)
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                list_.swap(swap_list_);
            }

            for (auto & item : *swap_list_)
            {
                func(item);
            }
            swap_list_->clear();
        }

    private:
        std::unique_ptr<std::list<T>> list_;
        std::unique_ptr<std::list<T>> swap_list_;
        std::mutex mutex_;
    };
}

#endif // USRV_SWAP_LIST_HPP
