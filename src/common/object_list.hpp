// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_OBJECT_LIST_HPP
#define USRV_OBJECT_LIST_HPP

#include <vector>
#include <mutex>

namespace usrv
{
    template<typename T>
    class ObjectList
    {
    public:
        static constexpr uint32_t INIT_OBJECT_LIST_NUM = 32;
        static constexpr uint32_t INIT_CUR_INDEX = 1;   // 0作为无效位    

    public:
        ObjectList() : size_(0),
            cur_index_(INIT_CUR_INDEX)
        {
            Resize(INIT_OBJECT_LIST_NUM);
        }

        std::shared_ptr<T> operator[](uint32_t index)
        {
            return At(index);
        }

    public:
        uint32_t Insert(T&& obj)
        {
            if (cur_index_ == size_)
            {
                Resize(size_ << 1);
            }

            auto index = indexs_[cur_index_++];
            objects_[index] = std::make_shared<T>(std::forward<T>(obj));

            return index;
        }

        std::shared_ptr<T> At(uint32_t index)
        {
            if (index >= size_)
            {
                return nullptr;
            }

            return objects_[index];
        }

        void Erase(uint32_t index)
        {
            if (index <= 0 || index >= size_ || objects_[index] == nullptr)
            {
                return;
            }

            objects_[index] = nullptr;
            indexs_[--cur_index_] = index;
        }

        uint32_t Size()
        {
            return size_;
        }

    private:
        void Resize(uint32_t size)
        {
            objects_.resize(size);
            indexs_.resize(size);
            for (auto i = size_; i < size; ++i)
            {
                indexs_[i] = i;
            }
            size_ = size;
        }

    private:
        std::vector<std::shared_ptr<T>> objects_;
        std::vector<uint32_t> indexs_;
        uint32_t size_;
        uint32_t cur_index_;
    };
}

#endif // USRV_OBJECT_LIST_HPP
