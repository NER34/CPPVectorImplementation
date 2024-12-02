#ifndef EPC_VECTOR
#define EPC_VECTOR

#include <cstdlib>
#include <utility>
#include <memory>
#include <math.h>

namespace epc
{
    template <typename T, size_t N>
    class vector
    {
    public:

        vector() noexcept {}

        vector(const vector& other)
        {
            reserve(other._capacity);
            copy_data(other.data(), data(), other._size);
            _size = other._size;
        }

        vector& operator=(const vector& other)
        {
            if (&other == this)
            {
                return *this;
            }

            // from here
            if (other._capacity > _capacity)
            {
                T* temp = (T*)::operator new(other._capacity * sizeof(T));

                destruct_data(data(), _size);

                ::operator delete(_long_data);

                _long_data = temp;
                _capacity = other._capacity;
            }
            else
            {
                destruct_data(data(), _size);
            }

            copy_data(other.data(), data(), other._size);

            _size = other._size;

            return *this;
        }

        ~vector()
        {
            clear();
            ::operator delete(_long_data);
        }

        T* data() { return _capacity != N ? _long_data : reinterpret_cast<T*>(_short_data); }
        const T* data() const { return _capacity != N ? _long_data : reinterpret_cast<const T*>(_short_data); }

        T& operator[](size_t index) { return *(data() + index); }
        const T& operator[](size_t index) const { return *(data() + index); }

        size_t capacity() const { return _capacity; }
        size_t size() const { return _size; }

        void reserve(size_t new_capacity)
        {
            if (new_capacity <= _capacity) return;

            T* temp = (T*)::operator new(new_capacity * sizeof(T));
            T* _data = data();

            for (size_t i = 0; i < _size; i++)
            {
                new (temp + i) T(std::move(*(_data + i)));
            }

            destruct_data(_data, _size);

            ::operator delete(_long_data);

            _long_data = temp;
            _capacity = new_capacity;
        }

        void push_back(const T& item)
        {
            if (_size == _capacity)
            {
                reserve(_capacity * 2);
            }

            construct_item(data() + _size, item);
            _size++;

        }

        void pop_back()
        {
            if (_size == 0)
            {
                return;
            }

            std::destroy_at(data() + _size - 1);
            _size--;
        }

        void clear()
        {
            destruct_data(data(), _size);
            _size = 0;
        }

        void swap(vector& other)
        {
            if (_capacity == N && other._capacity == N)
            {
                swap_same_buffers(data(), other.data(), _size, other._size);

                const size_t temp = other._size;
                other._size = _size;
                _size = temp;
            }
            else
            {
                if (_capacity != N || other._capacity != N)
                {
                    if (_capacity == N)     reserve(other._capacity);
                    else                    other.reserve(_capacity);
                }

                {
                    T* temp = other._long_data;
                    other._long_data = _long_data;
                    _long_data = temp;
                }
                {
                    const size_t temp = other._capacity;
                    other._capacity = _capacity;
                    _capacity = temp;
                }
                {
                    const size_t temp = other._size;
                    other._size = _size;
                    _size = temp;
                }
            }
        }

    private:

        void swap_same_buffers(T* buffer_1, T* buffer_2, size_t size_1, size_t size_2)
        {
            T* long_buf = size_1 >= size_2 ? buffer_1 : buffer_2;
            T* short_buf = size_1 < size_2 ? buffer_1 : buffer_2;

            const size_t short_size = __min(size_1, size_2);
            const size_t long_size = __max(size_1, size_2);

            for (size_t i = 0; i < short_size; i++)
            {
                std::swap(*(short_buf + i), *(long_buf + i));
            }

            for (size_t i = short_size; i < long_size; i++)
            {
                new (short_buf + i) T(std::move(*(long_buf + i)));
                std::destroy_at(long_buf + i);
            }
        }

        template <typename ... Ts>
        void construct_item(T* _buffer, Ts&& ... args)
        {
            new (_buffer) T(std::forward<Ts>(args)...);
        }

        void copy_data(const T* from, T* to, size_t buffer_size)
        {
            for (size_t i = 0; i < buffer_size; i++)
            {
                construct_item(to + i, *(from + i));
            }
        }

        void destruct_data(T* buffer, size_t buffer_size)
        {
            for (size_t i = 0; i < buffer_size; i++)
            {
                std::destroy_at(buffer + i);
            }
        }

        size_t          _capacity = N;
        size_t          _size = 0;
        T* _long_data = nullptr;

        unsigned char   _short_data[N * sizeof(T)];

    };
}

#endif


