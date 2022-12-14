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
#include <thread>
#include <vector>
#include "db.hpp"

#define MAXFDS 10000
#define MAX_BUF_LENGTH 1024

typedef enum {
    AddCategory = 0,
    AddProduct,
    GetCategory,
    GetProduct,
    RemoveCategory,
    RemoveProduct
} Action;

typedef enum {
    IO_READ = 0,
    IO_WRITE,
    IO_NO_READ_WRITE,
    IO_CLOSED
} IOEvent;

typedef struct {
    IOEvent event;
    std::string buff;
    int fd;

    void reset() {
        event = IOEvent::IO_CLOSED;
        buff.clear();
        fd = -1;
    }
} ConnectionState;

class Server {
private:
    int m_ListenSocket;
    int m_EpollFd;
    int m_Threads;
    std::unique_ptr<epoll_event[]> m_Events;
    std::unique_ptr<ConnectionState[]> m_ConnectionStates;

private:
    void WaitIOEvents(std::shared_ptr<Db> db);
    void SetIOEvent(ConnectionState* connection_state, int operation);
    void Read(std::shared_ptr<Db> db, ConnectionState* p_ConnectionState);
    void Write(ConnectionState* p_ConnectionState);
    void Handler(std::shared_ptr<Db> db, std::string request, 
            ConnectionState* p_ConnectionState);

public:
    Server(int portnum, int threads = 2);
    ~Server();
    void Run();
};
