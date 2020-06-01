// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_OBJECT_POOL_HPP
#define USRV_OBJECT_POOL_HPP

#include <list>
#include <mutex>
#include <memory>
#include <functional>

namespace usrv
{
    template<typename Object, typename... Param>
    class ObjectPool
    {
    public:
        using DelType = std::function<void(Object *)>;
        using ObjPtr = std::unique_ptr<Object, DelType>;
        using Constructor = std::function<void(ObjPtr &, Param...)>;
        using Destructor = std::function<void(std::unique_ptr<Object> &)>;

    public:
        ObjectPool(uint32_t init_num = 0, int32_t max_num = -1) : init_num_(init_num),
            max_num_(max_num),
            init_func_(nullptr),
            reuse_func_(nullptr),
            release_func_(nullptr)
        {
        }

    public:
        ObjPtr Get(Param&&... param)
        {
            ObjPtr ptr = nullptr;

            {
                std::lock_guard<std::mutex> lock(mutex_objects_);
                if (!objects_.empty())
                {
                    ptr = ObjPtr(objects_.front().release(), del_func_);
                    objects_.pop_front();
                }
            }

            if (ptr == nullptr)
            {
                ptr = CreateObject(std::forward<Param>(param)...);
            }
            else
            {
                if (reuse_func_ != nullptr)
                {
                    reuse_func_(ptr, std::forward<Param>(param)...);
                }
            }

            return std::move(ptr);
        }

        void Init(Param&&... param)
        {
            for (uint32_t i = 0; i < init_num_; ++i)
            {
                CreateObject(std::forward<Param>(param)...);
            }
        }

        void SetInitFunc(Constructor func)
        {
            init_func_ = func;
        }

        void SetReuseFunc(Constructor func)
        {
            reuse_func_ = func;
        }

        void SetReleaseFunc(Destructor func)
        {
            release_func_ = func;
        }

    private:
        ObjPtr CreateObject(Param&&... param)
        {
            auto ptr = ObjPtr(new Object(), del_func_);

            if (init_func_ != nullptr)
            {
                init_func_(ptr, std::forward<Param>(param)...);
            }

            return std::move(ptr);
        }

    private:
        const DelType del_func_ = [this](Object * obj)
        {
            auto ptr = std::unique_ptr<Object>(obj);

            if (release_func_ != nullptr)
            {
                release_func_(ptr);
            }

            std::lock_guard<std::mutex> lock(mutex_objects_);

            if (max_num_ >= 0 && (int32_t)objects_.size() >= max_num_)
            {
                return;
            }

            objects_.emplace_back(std::move(ptr));
        };

    private:
        std::list<std::unique_ptr<Object>> objects_;
        std::mutex mutex_objects_;

        uint32_t init_num_;
        int32_t max_num_;

        Constructor init_func_;
        Constructor reuse_func_;
        Destructor release_func_;
    };
}

#endif // USRV_OBJECT_POOL_HPP
