#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <atpa/inet.h>
#include <netinet/in.h>

//net
#include <net/if_arp.h>
#include <net/if.h>
//sys
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>

//c++
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <vector>

#include "ipc.h"
#include "scopeguard.h"



ConnectionUDS::ConnectionUDS(Poller *p):ConnectionBase(p)
{
}

ConnectionUDS::~ConnectionUDS()
{
    close();
}

void ConnectionUDS::notify(int error_code, int transferred_cnt, uintptr_t hint, PollEvent *pevt)
{

}

void ConnectionUDS::read(void *pbuff, const int len, int *lp_cnt = NULL)
{

}

void ConnectionUDS::write(const void *pbuff, const int len)
{

}

void ConnectionUDS::read(OS_HANDLE& oshd)
{

}

void ConnectionUDS::write(OS_HANDLE& oshd)
{

}

void ConnectionUDS::create(const std::string& serverName,std::tuple<std::sting, std::string, int> accessAttrs)
{
    if(m_state != state::EMPTY)
        std::cerr << "state is not EMPTY when create()" << std::endl;
    
    // create a UNIX domain stream socket 
    if(m_fd = socket(AF_UNIX, SOCKET_STREAM, 0) < 0)
        std::cerr << "ipc_connect_linux_UDS : socket() failed" << std::endl;
    
    auto g1 = makeScopeGuard([&] { close(); }); 

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;

    std::string sun_path = serverName;
    if(sun_path.empty()) {
        //client socket do not have server name, build one for it.
        std::ostringstream stringstream;
        stringstream << CLI_PATH;
        stringstream << std::setw(5) << std::setfill('0') << getpid();
        sun_path = stringstream.str();
    }

    ::unlink(sun_path.c_str());

    int cnt = StringHelp::copyStringSafe(addr.sun_path, sun_path);
    int len = offsetof(sockaddr_un, sun_path) + cnt +1;

    if(::bind(m_fd, (struct sockaddr*)&addr, len) < 0)
        std::cerr << "ipc_connection_linux_UDS : bind() failed" << std::endl;

    m_name = sun_path;

    setConnectionAccessAttribute(serverName, accessAttrs);

    g1.dismiss();

}

bool ConnectionUDS::connect(const std::string& serverName)
{
    if(m_state != state::EMPTY)
        std::cerr << "state is not EMPTY when connect()" << std::endl;

    // create a Unix Domain Stream socket
    if ((m_fd = socket(AF_UNIX, SOCKET_STREAM, 0)) < 0)
        std::cerr << "ipc_connection_linux_UDS : socket() failed." << std::endl;

    auto g1 = makeScopeGuard([&] { close(); });

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    // fill socket address structure with server's address.
    int cnt = StringHelp::copyStringSafe(addr.sun_path, serverName);
    int len = offsetof(struct sockaddr_un, sun_path) + cnt + 1;

    
}

void ConnectionUDS::listen()
{
    if(::listen(m_fd. m_numListen) < 0)
        std::cerr << "ipc_connection_linux_UDS : listen() failed" << std::endl;

    m_state = state::LISTEN;
    m_poller->add(this);

}

void ConnectionUDS::accept(ConnectionBase *listener)
{
    if(m_state != state::EMPTY)
        std::cerr << "state is not EMPTY when connect()" << std::endl;
    struct sockaddr_un addr;

    if(listener == nullptr)
        std::cerr << "listener is null" << std::endl;

    ConnectionUDS *plis = dynamic_cast<ConnectionUDS*>(listener);
    if(plis == nullptr)
        std::cerr << "listener is not ipc_connection_linux_UDS" << std::endl;

    socklen_t len = sizeof(addr);
    if((m_fd = ::accept(listener->nativeHandle(), (struct sockaddr*)&addr, &len)) < 0)
        std::cerr << "ipc_connection_linux_UDS : accpet() failed" << std::endl;
        
    auto g1 = makeScopeGuard([&]{close()});

    m_state = state::CONN;
    m_poller->add(this);

    g1.dismiss();
}

virtual bool connected(void);

bool set_bolck_mode(bool makeBlocking = true);
void setConnectionAccessAttribute(const std::string& serverName, std::tuple<std:;string, std::string, int>accessAttrs);

void blockPoller();
void unblockPoller();

constexpr static const char *CLI_PATH = "/var/tmp";
constexpr static const int m_numListen = 5;

std::atomic<bool> m_poller_blocked(false);
int m_fd = -1;
std::string m_name = "";  //IPC name

state m_state = state::EMPTY;

std::mutex m_mutex;
