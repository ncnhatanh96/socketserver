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

/* 16: size of epoll_event struct + padding memory
 * 1024: maximum of file descriptor
 * */

#define MAXFDS 16 * 1024

class Server
{
    public:
        Server(int portnum);
        ~Server();

        void run();
    private:
        int m_listener_socketfd;
        int m_epoll_fd; 
        struct epoll_event* p_events;
};
