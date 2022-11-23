#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.hpp"
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <memory>
#include <iostream>
#include <cstring>
#include "db.hpp"
#define SENDBUF_SIZE 1024
#define MAXFDS 10000

class Action {
public:
    Action() = default;
    ~Action() = default;

    enum IOEvent_Type: int {
        IO_EVENT_R = 1,
        IO_EVENT_W,
        IO_EVENT_RW,
        IO_EVENT_NO_RW
    };

    void setIOEventType(IOEvent_Type ioEvent) {
        ioEventType = ioEvent;
        return;
    }

    IOEvent_Type getIOEventType() {
        return ioEventType;
    }

    void setFd(int sockfd) {
        fd = sockfd;
    }

    int getFd() {
        return fd;
    }

private:
    IOEvent_Type ioEventType = IO_EVENT_R;
    int fd;
};

class Server {
public:
    Server(int portnum);
    ~Server();
    void run();

private:
    int m_listener_socketfd;
    int m_epoll_fd;
    std::unique_ptr<epoll_event[]> m_Events;
    std::unique_ptr<Action[]> m_Actions;
    Db db;

private:
    void newConnection();
    void waitEvents();
    void setIOEvent(Action* action, int operation);
};
