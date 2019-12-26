#ifndef __HANDLER_MAP_H
#define __HANDLER_MAP_H

#include <type_traits>
#include <stdlib.h>
#include <string.h>

namespace co
{

template <typename ValueType>
class HandlerMap
{
    static_assert(std::is_pointer<ValueType>::value, "valuetype must be pointer");
public:
    HandlerMap()
    {
        m_data = static_cast<ValueType **>(malloc(m_size * sizeof(ValueType *)));
        memset(m_data, 0, m_size * sizeof(ValueType *));
    }
    ~HandlerMap()
    {
        for(int i = 0; i < m_size; i++)
            if(m_data[i])
                free(m_data[i]);
        free(m_data);
    }
    void insert(int fd, ValueType val)
    {
        if(fd < 0 || fd >= m_size * m_size)
            throw "wrong fd";

        int idx1 = fd / m_size;
        int idx2 = fd & (m_size - 1);
        if(!m_data[idx1])
        {
            m_data[idx1] = static_cast<ValueType *>(malloc(m_size * sizeof(ValueType *)));
            memset(m_data[idx1], 0, m_size * sizeof(ValueType *));
        }
        m_data[idx1][idx2] = val;
    }

    ValueType get(int fd)
    {
        if(fd < 0 || fd >= m_size * m_size)
            throw "wrong fd";

        int idx1 = fd / m_size;
        int idx2 = fd & (m_size - 1);
        if(!m_data[idx1])
            return nullptr;
    
        return m_data[idx1][idx2];
    }

    void erase(int fd)
    {
        if(fd < 0 || fd >= m_size * m_size)
            throw "wrong fd";

        int idx1 = fd / m_size;
        int idx2 = fd & (m_size - 1);
        if(!m_data[idx1])
            throw "no data";

        m_data[idx1][idx2] = nullptr;
    }
private:
    static constexpr int m_size{1024};
    ValueType **m_data{nullptr};
};

}//namespace co

#endif
