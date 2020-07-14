// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_OBJECT_LIST_HPP
#define USRV_OBJECT_LIST_HPP

#include <vector>

namespace usrv
{
    template<typename T>
    class ObjectList
    {
    public:
        static constexpr size_t INIT_SIZE = 32;

    public:
        ObjectList() : size_(0)
        {
            Resize(INIT_SIZE);
        }

        std::shared_ptr<T> operator[](size_t id)
        {
            if (id >= objects_.size())
            {
                return nullptr;
            }
            return objects_[id];
        }

        std::shared_ptr<T> Find(size_t index)
        {
            auto id = Index2ID(index);
            if (id == -1)
            {
                return nullptr;
            }
            return operator[](id);
        }

    public:
        size_t Size()
        {
            return size_;
        }

        size_t Insert(T&& obj)
        {
            auto ptr = std::make_shared<T>(std::forward<T>(obj));
            return Insert(std::move(ptr));
        }

        size_t Insert(std::shared_ptr<T> ptr)
        {
            if (size_ == objects_.size())
            {
                Resize(objects_.size() << 1);
            }

            auto id = indexs_[size_++];
            objects_[id] = std::move(ptr);

            return id;
        }

        void Erase(size_t id)
        {
            if (id >= objects_.size() || objects_[id] == nullptr)
            {
                return;
            }

            objects_[id] = nullptr;
            indexs_[--size_] = id;
        }

        void EraseByIndex(size_t index)
        {
            auto id = Index2ID(index);
            if (id == -1)
            {
                return;
            }
            Erase(id);
        }

    private:
        void Resize(size_t size)
        {
            objects_.resize(size);
            indexs_.resize(size);
            for (auto i = size_; i < size; ++i)
            {
                indexs_[i] = i;
            }
        }

        size_t Index2ID(size_t index)
        {
            if (index >= size_)
            {
                return -1;
            }
            return indexs_[index];
        }

    private:
        std::vector<std::shared_ptr<T>> objects_;
        std::vector<size_t> indexs_;
        size_t size_;
    };
}

#endif // USRV_OBJECT_LIST_HPP
