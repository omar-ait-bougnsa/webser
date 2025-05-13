#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <cerrno>

class EpollManager
{
private:
    int _epollFd;
    static const int MAX_EVENTS = 10;

public:
    EpollManager();
    ~EpollManager();

    void addFd(int fd);
    void removeFd(int fd);
    std::vector<int> waitEvents(int timeoutMs = 5000);
    int getEpollFd() const;
};




#endif
