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

typedef enum {
    IO_READ = 0,
    IO_WRITE,
    IO_NO_READ_WRITE
} IOEvent;

typedef struct {
    IOEvent event;
    std::string buff;
    int fd;

    void reset() {
        event = IOEvent::IO_READ;
        buff = "";
        fd = -1;
    }
} ConnectionState;

class Server {
private:
    int m_ListenSocket;
    int m_EpollFd;
    std::unique_ptr<epoll_event[]> m_Events;
    std::unique_ptr<ConnectionState[]> m_ConnectionStates;
    Db db;

private:
    void WaitEvents();
    void SetIOEvent(ConnectionState* connection_state, int operation);

public:
    Server(int portnum);
    ~Server();
    void Run();
};
