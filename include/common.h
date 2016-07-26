#ifndef __CRACK97_COMMON_H__
#define __CRACK97_COMMON_H__


#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <map>
#include <string>


namespace e7 {
namespace common {
namespace deny_copyable_ {
    class deny_copyable;
}
typedef deny_copyable_::deny_copyable deny_copyable;

namespace object_ {
    class object;
}
typedef object_::object object;

template <typename TYPE> class array;

template <typename base, typename derived> class inheritance_ship;
template <typename TYPE> class smart_pointer;

namespace fdraii_ {
    class fdraii;
}
typedef fdraii_::fdraii fdraii;

template <typename TYPE> class raii;
} // end of namespace e7::common


template <typename base, typename derived> class e7::common::inheritance_ship
{
public:
    /* 检测继承关系，真表示继承关系成立 */
    bool operator ()(void) const
    {
        return (sizeof(check(static_cast<base const*>(0))) == \
            sizeof(check(static_cast<derived const*>(0))));
    }

    operator bool(void) const
    {
        return (*this)();
    }

private:
    /* type为无用模板形参，因为嵌套类模板只能偏特化 */
    template <int n, typename type=void> class __size_box__
    {
    private:
        __size_box__<n-1, type> __box1__;
        __size_box__<n-1, type> __box2__;
    };

    template <typename type> class __size_box__<0, type>
    {
    private:
        char __c__;
    };

private:
    __size_box__<0> check(base const*) const;
    __size_box__<1> check(...) const;

};


class e7::common::deny_copyable_::deny_copyable
{
protected:
    explicit deny_copyable(void) {}
    virtual ~deny_copyable(void) {}
    virtual int init(void)
    {
        return 0;
    }

private:
    deny_copyable(const deny_copyable &);
    deny_copyable const &operator =(const deny_copyable &);
}; // end of e7::common::deny_copyable_::deny_copyable


// 从object继承的对象用于智能指针，而指针是无需拷贝构造的，或者拷贝智能指针对象
class e7::common::object_::object : public e7::common::deny_copyable
{
     template <typename TYPE> friend class e7::common::smart_pointer;

public:
    explicit object(void)
        : __ref_count__(0), __count_mutex__(NULL)
    {
        __count_mutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__count_mutex__, NULL);
    }
    explicit object(object const &other)
        : __ref_count__(0), __count_mutex__(NULL)
    {
        __count_mutex__ = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(__count_mutex__, NULL);
    }
    virtual ~object(void)
    {}

private:
    void *operator new[](size_t size);
    void operator delete[](void *p);

private:
    // just for smart_pointer
    void __ref_increase__(void)
    {
        ++__ref_count__;
    }

    intptr_t __get_ref_count__(void) const
    {
        return __ref_count__;
    }

    void __ref_decrease__(void)
    {
        --__ref_count__;
    }

private:
    intptr_t __ref_count__;
    pthread_mutex_t *__count_mutex__;
}; // end of e7::common::object_::object


template <typename TYPE>
class e7::common::array : public e7::common::object_::object
{
     friend class e7::common::smart_pointer< array<TYPE> >;

public:
    array<TYPE>(TYPE *obj_array, size_t n)
    {
        this->core.reserve(n);

        for (size_t i = 0; i < n; ++i) {
            this->core.push_back(&obj_array[i]);
        }
    }

    virtual ~array<TYPE>(void)
    {
        delete[] this->core[0];
    }

    TYPE *at(size_t i)
    {
        if (i < this->core.size()) {
            return this->core[i];
        } else {
            return NULL;
        }
    }

private:
    std::vector<TYPE *> core;
}; // end of template class e7::common::array


class e7::common::fdraii_::fdraii : public e7::common::deny_copyable
{
public:
    explicit fdraii(int fd) : rsc(fd) {}
    virtual ~fdraii(void)
    {
        static_cast<void>(::close(rsc));
    }

    int get(void) const
    {
        return rsc;
    }

private:
    int rsc;
}; // end of e7::common::fdraii


template <typename T>
class e7::common::raii : public e7::common::deny_copyable
{
public:
    explicit raii<T>(T *obj) : rsc(obj) {}
    virtual ~raii<T>(void)
    {
        delete this->rsc;
    }

    T *get(void) const
    {
        return rsc;
    }

private:
    T *rsc;
}; // end of e7::common::raii
} // end of namespace e7
#endif // __CRACK97_COMMON_H__
