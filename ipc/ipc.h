#ifndef  _IPC_H_
#define  _IPC_H_

#include <iostream>
#include <string>
#include <memory>


#include "../singleton/singleton.h"

typedef int OS_HANDLE;

class ConnectionBase;

struct PollEvent
{
    enum event
    {
        NONE = 0,
        POLLIN = 1,
        POLLHUP = 2
    }
    ConnectionBase *pconn = nullptr;
};

class PollBlocker
{
    ConnectionBase *m_pc;
    PollBlocker(ConnectionBase *pc);
    ~PollBlocker();
    friend class ConnectionBase;
};

class ConnectionBase: public NonCopyable
{
public:
    virtual ~ConnectionBase();
    virtual OS_HANDLE nativeHandle() = 0;

    virtual void read(void *pbuff, const int len, int *lp_cnt = NULL) = 0;
    virtual void write(const void *pbuff, const int len) = 0;

    // can be used on duplicate fd() on Linux.
    virtual void read(OS_HANDLE& oshd) = 0;
    virtual void write(OS_HANDLE& oshd) = 0;

    virtual void create(const std::string& serverName, std::tuple<std::string, std::string, int>accessAttrs) = 0;
    virtual bool connect(const std::string& serverName) = 0;
    virtual void listen() = 0;
    virtual void accept(ConnectionBase* listener) = 0;

    template <typename Conn>
    ConnectionBase* accept(void)
    {
        ConnectionBase* pconn = nullptr;
        try {
            pconn = new Conn(this->m_poller);
        } catch(...) {
            std::cerr << "Error: bad alloc exception occured." << std::endl;
            return nullptr;
        }

        pconn->accept(this);
        return pconn;
    }

    //close underlying resource
    virtual void close(void) = 0;
    virtual bool connected(void) = 0;
    
    friend class Poller;

    std::shared_ptr<PollBlocker> makeBlockerPILLIN()
    {
        this->blockPoller();
        return std::shared_ptr<PollBlocker>(new PollBlocker(this));
    }

    virtual void blockPoller() {}
    virtual void unblockPoller() {}

    static void close(int connfd);
    static ConnectionBase* getConnectionById(int connfd);

    int getConnIf(void)
    {
        return m_simulateFD;
    }
    void serUser(std::string user)
    {
        m_user = std::move(user);
    }

protected:
    friend class PollBlocker;

    virtual void notify(int error_code, int transferred_cnt, uintptr_t hint, PollEvent *pevt) = 0;

    static const int IPC_CONN_FD_MAX = 2048;
    static std::mutex m_fd_mutex;
    static ConnectionBase *m_fd_mao[IPC_CONN_FD_MAX];

    ConnectionBase(Poller *p);

    Poller *m_poller;
    
    int m_simulateFD;
    std::string m_user;

};


// Unix Domain Sockets
class class ConnectionUDS : public ConnectionBase
{
public:
    ConnectionUDS(Poller *p = nullptr);
    ~ConnectionUDS();

    virtual void notify(int error_code, int transferred_cnt, uintptr_t hint, PollEvent *pevt);

    virtual OS_HANDLE nativeHandle()
    {
        return m_fd;
    }

    virtual void read(void *pbuff, const int len, int *lp_cnt = NULL);
    virtual void write(const void *pbuff, const int len);

    virtual void read(OS_HANDLE& oshd);
    virtual void write(OS_HANDLE& oshd);

    virtual void create(const std::string& serverName,std::tuple<std::sting, std::string, int> accessAttrs);
    virtual bool connect(const std::string& serverName);
    virtual void listen();
    virtual void accept(ConnectionBase *listener);

    virtual bool connected(void);

protected:

    bool set_bolck_mode(bool makeBlocking = true);
    void setConnectionAccessAttribute(const std::string& serverName, std::tuple<std:;string, std::string, int>accessAttrs);

    void blockPoller();
    void unblockPoller();

    constexpr static const char *CLI_PATH = "/var/tmp";
    constexpr static const int m_numListen = 5;

    std::atomic<bool> m_poller_blocked(false);
    int m_fd = -1;
    std::string m_name = "";  //IPC name
    
    enum class state{
        EMPTY=0,
        LISTEN,
        CONN,
        DISCONN
    };

    state m_state = state::EMPTY;

    std::mutex m_mutex;
};
















































#endif  //_IPC_H_
