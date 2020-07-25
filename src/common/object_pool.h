// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_OBJECT_POOL_HPP
#define USRV_OBJECT_POOL_HPP

#include <list>
#include <memory>
#include <functional>

namespace usrv
{
    template<typename T>
    class ObjectPool
    {
    public:
        static constexpr size_t INIT_SIZE = 32;
        static constexpr size_t MAX_SIZE = 1024;

    public:
        ObjectPool(size_t init_num = INIT_SIZE, size_t max_num = MAX_SIZE) : max_num_(max_num),
            destructor_(nullptr)
        {
            for (auto i = 0; i < init_num; ++i)
            {
                objects_.emplace_back(new T());
            }
        }

    public:
        std::shared_ptr<T> Get()
        {
            T * obj = nullptr;

            if (!objects_.empty())
            {
                obj = objects_.front();
                objects_.pop_front();
            }

            if (!obj)
            {
                obj = new T();
            }

            return std::move(std::shared_ptr<T>(obj, del_func_));
        }

        void SetDestructor(std::function<void(T * obj)> func)
        {
            destructor_ = func;
        }

    private:
        const std::function<void(T * obj)> del_func_ = [this](T * obj)
        {
            if (destructor_ != nullptr)
            {
                destructor_(obj);
            }

            if (objects_.size() >= max_num_)
            {
                delete obj;
                return;
            }

            objects_.emplace_back(obj);
        };

    private:
        std::list<T *> objects_;
        size_t max_num_;

        std::function<void(T * obj)> destructor_;
    };
}

#endif // USRV_OBJECT_POOL_HPP
