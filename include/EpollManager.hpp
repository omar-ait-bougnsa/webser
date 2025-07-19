#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <cerrno>

struct EpollEvent {
    int fd;
    uint32_t events;
};

class EpollManager
{
private:
    int _epollFd;
    static const int MAX_EVENTS = 10;

public:
    EpollManager();
    ~EpollManager();

    void                    createEpollManger();
    void                    addFd(int fd);
    void                    removeFd(int fd);
    std::vector<EpollEvent> waitEvents(int timeoutMs = 5000);
    void                    modifyFd(int fd, uint32_t events);
    int                     getEpollFd() const;
};




#endif
