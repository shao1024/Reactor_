#pragma once

#include <string>
#include <iostream>

class Buffer
{
private:
    std::string buf_;

public:
    Buffer();
    ~Buffer();

    // 把数据追加到buf_中
    void append(const char* data, size_t size);
    // 把数据追加到buf_中，附加报文头部
    void appendwithhead(const char *data,size_t size);
    // 从buf_的pos开始，删除nn个字节，pos从0开始
    void erase(size_t pos, size_t nn);
    // 返回buf_的大小
    size_t size();
    // 返回buf_的首地址 
    const char* data();
    // 清空buf_
    void clear();


};





