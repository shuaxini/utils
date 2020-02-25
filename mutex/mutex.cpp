#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <string.h>

#include "mutex.h"

GlobalMutex::GlobalMutex(std::string name, bool closeOnExec) :
    m_name(std::move(name)),
    m_fd(0)
{
    if (m_name.empty()) {
        return;
    }

    m_fd = open(m_name.c_str(), O_CREAT, 0660);

    if (m_fd < 0) {
        printf("Error: Open GlobalMutex %s failed. errno = %d [%s]", m_name.c_str(), errno, strerror(errno));
        return;
    }

    /* set the file locked immediately if needed, default is not*/
    if (closeOnExec) {
        int flags = fcntl(m_fd, F_GETFD) | FD_CLOEXEC;
        fcntl(m_fd, F_SETFD, flags);
    }
}

GlobalMutex::~GlobalMutex()
{
    if (m_fd > 0) {
        close(m_fd);
        m_fd = 0;
    }
}

/*
 *Lock for long.
 * */
bool GlobalMutex::lock() {
    AutoMutex autoLock(m_mutex);

    if (m_fd <= 0) {
        printf("Error: GlobalMutex %s is not initialized.", m_name.c_str());
        return false;
    }

    if (flock(m_fd, LOCK_EX) < 0) {
        printf("Error: Lock GlobalMutex(%s) failed. errno = %d [%s]", m_name.c_str(), errno, strerror(errno));
        return false;
    }

    return true;
}

/*
 *Lock for a period of time.
 * */
bool GlobalMutex::lock(long timeoutMilliseconds) {

    AutoMutex autoLock(m_mutex);

    // Because there is no timeout lock in flock in Linux,
    // this implementation is a workaround for this function.
    if (m_fd <= 0) {
        printf("Error: GlobalMutex %s is not initialized.", m_name.c_str());
        return false;
    }

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    while (flock(m_fd, LOCK_EX | LOCK_NB) < 0) {
        std::chrono::steady_clock::time_point n = std::chrono::steady_clock::now();
        std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(n - now);

        if (d.count() >= timeoutMilliseconds) {
            /* Time out */
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return true;
}

bool GlobalMutex::trylock()
{
    AutoMutex autoLock(m_mutex);

    if (m_fd <= 0) {
        printf("Error: GlobalMutex %s is not initialized.", m_name.c_str());
        return false;
    }

    if (flock(m_fd, LOCK_EX | LOCK_NB) < 0) {
        return false;
    }

    return true;
}

void GlobalMutex::unlock()
{
    AutoMutex autoLock(m_mutex);

    flock(m_fd, LOCK_UN);
}
