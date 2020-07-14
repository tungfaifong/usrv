// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_BUFFER_HPP
#define USRV_BUFFER_HPP

#include <cstring>

namespace usrv
{
    class Buffer
    {
    public:
        Buffer() :size_(0),
            data_(nullptr)
        {
        }

        Buffer(const char * data, size_t size) :size_(0),
            data_(nullptr)
        {
            Reset(data, size);
        }

        Buffer(Buffer & r) :size_(0)
        {
            Reset(r.data_, r.size_);
        }

        Buffer(Buffer && r) :size_(r.size_),
            data_(r.data_)
        {
            r.data_ = nullptr;
        }

        ~Buffer()
        {
            if (data_ != nullptr)
            {
                delete[] data_;
            }
        }

        void ResetData()
        {
            memset(data_, 0, size_ + 1);
        }

    public:
        void Reset(const char * data, size_t size)
        {
            Resize(size);
            ResetData();
            memcpy(data_, data, size_);
        }

        void Set(const char * data, size_t size, size_t offset = 0)
        {
            offset = offset > size_ ? size_ : offset;
            size = offset + size > size_ ? size_ - offset : size;
            memcpy(data_ + offset, data, size);
        }

        void Resize(size_t size)
        {
            if (data_ != nullptr)
            {
                delete[] data_;
            }

            size_ = size;
            data_ = new char[size_ + 1];
        }

        size_t Size() const { return size_; }
        const char * Data() const { return data_; }

    private:
        size_t size_;
        char * data_;
    };

    template<size_t SIZE>
    class FixedBuffer
    {
    public:
        FixedBuffer() :size_(0)
        {
            ResetData();
        }

        FixedBuffer(const char * data, size_t size) :size_(0)
        {
            Reset(data, size);
        }

        FixedBuffer(FixedBuffer & r) :size_(0)
        {
            Reset(r.data_, r.size_);
        }

        void ResetData()
        {
            memset(data_, 0, SIZE + 1);
        }

    public:
        void Reset(const char * data, size_t size)
        {
            Resize(size);
            ResetData();
            memcpy(data_, data, size_);
        }

        void Set(const char * data, size_t size, size_t offset = 0)
        {
            offset = offset > size_ ? size_ : offset;
            size = offset + size > size_ ? size_ - offset : size;
            memcpy(data_ + offset, data, size);
        }

        void Resize(size_t size) { size_ = size > SIZE ? SIZE : size; }

        size_t Size() const { return size_; }
        const char * Data() const { return data_; }

    private:
        size_t size_;
        char data_[SIZE + 1];
    };
}

#endif // USRV_BUFFER_HPP
