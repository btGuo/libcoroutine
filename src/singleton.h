#ifndef __SINGLETON_H
#define __SINGLETON_H

namespace co
{

template <typename T>
class Singleton
{
public:
    static T &getInstance()
    {
        static T instance;
        return instance;
    }
};

}

#endif
