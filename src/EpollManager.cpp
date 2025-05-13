#include "../include/EpollManager.hpp"

EpollManager::EpollManager()
{
    _epollFd = epoll_create(1);
    if (_epollFd == -1)
        throw std::runtime_error("Failed to create Epoll Manager");
}

EpollManager::~EpollManager()
{
    if (_epollFd >= 0)
        close(_epollFd);
}

void EpollManager::addFd(int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
        throw std::runtime_error("Failed to add socket to Epoll Manager");
}

void EpollManager::removeFd(int fd)
{
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw std::runtime_error("Failed to remove socket from Epoll Manager");
}
int EpollManager::getEpollFd() const
{
    return _epollFd;
}

std::vector<int> EpollManager::waitEvents(int timeoutMs)
{
    int nfds;
    std::vector<int> readyFds;
    struct epoll_event events[MAX_EVENTS];
    while (true)
    {

        nfds = epoll_wait(_epollFd, events, MAX_EVENTS, timeoutMs);
        if (nfds == -1)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("Failed Epoll wait");
        }
        for (int i = 0; i < nfds; i++)
            readyFds.push_back(events[i].data.fd);
        return readyFds;
    }
}