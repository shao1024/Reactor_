#pragma once

#include <iostream>
#include <string>
#include <time.h>

// 时间戳
class Timestamp
{
private:
    // 整数表示的时间
    time_t secsinceepoch_;
public:
    // 用当前时间初始化对象
    Timestamp();
    // 用一个整数表示的时间初始化对象
    Timestamp(int64_t secsinceepoch);
    ~Timestamp();

    // 返回当前时间的Timestamp对象
    static Timestamp now();

    // 返回整数表示的时间
    time_t toint() const;
    // 返回字符串表示的时间
    std::string tostring() const;

};

