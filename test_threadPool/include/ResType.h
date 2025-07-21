#ifndef RESTYPE_H
#define RESTYPE_H
#include <memory>
// 构造Any类
class Base
{
public:
    virtual ~Base() = default;
};

template<typename T>
class Derive : public Base
{
public:
    Derive();
    Derive(T data):data_(data)
    {}
    T data_;
};
class Any
{
public:
    Any() = default;
    ~Any() = default;
    // 避免拷贝构造造成的成员变量base_被拷贝
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    template<typename T>
    Any(T data)
    {
        base_ = std::make_unique<Derive<T>>(data);
    }

    template<typename T>
    T get_res()
    {
        // return base_-> 基类指针没办法访问派生类成员变量，所以先转成派生类指针
       // Derive<T>* res = dynamic_cast<Derive<T>*>(base_); 不能直接对智能指针dynamic_cast
        Derive<T>* res = dynamic_cast<Derive<T>*>(base_.get());
        if(res == nullptr)
        {
            throw "type is unmatch!";
        }
        return res->data_;
    }
private:
    std::unique_ptr<Base> base_;
};

#endif