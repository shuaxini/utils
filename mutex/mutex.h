#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <mutex>
#include <string>

class Mutex
{
public:
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    bool tryLock() { return m_mutex.try_lock(); }

private:
    std::mutex m_mutex;
};


class AutoMutex
{
public:
    AutoMutex(Mutex& mutex) : m_mutex(&mutex) {
        m_mutex->lock();
    };

    ~AutoMutex() {
        m_mutex->unlock();
    };

private:
    Mutex* m_mutex;
};


class GlobalMutex
{
public:
    GlobalMutex(std::string name, bool closeOnExec = false);
    virtual ~GlobalMutex();

    bool lock();
    bool lock(long timeoutMilliseconds);
    bool trylock();
    void unlock();

private:
    Mutex       m_mutex;
    std::string m_name;
    int         m_fd;
};


#endif //_MUTEX_H_
