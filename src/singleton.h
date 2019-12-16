#ifndef __SINGLETON_H
#define __SINGLETON_H

namespace co
{

#define SingletonDeclare(type) friend class Singleton<type>

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
