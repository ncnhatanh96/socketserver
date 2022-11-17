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

#define SENDBUF_SIZE 1024

typedef enum {
    INITIAL_ACK = 0,
    WAIT_FOR_MSG,
    IN_MSG
} ProcessingState; //connection state

typedef struct {
  bool want_read;
  bool want_write;
} fd_status_t;

const fd_status_t fd_status_R = {.want_read = true, .want_write = false};
const fd_status_t fd_status_W = {.want_read = false, .want_write = true};
const fd_status_t fd_status_RW = {.want_read = true, .want_write = true};
const fd_status_t fd_status_NORW = {.want_read = false, .want_write = false};

typedef struct {
    ProcessingState state;
    uint8_t sendbuf[SENDBUF_SIZE];
    int sendbuf_end;
    int sendptr;
} peer_state_t; //client state

class Action {
    public:
        Action() { 
            return; 
        }
        ~Action() {
            return;
        }
};

class Server {

public:
    Server(int portnum);
    ~Server(); 
    void run();

private:
    const int MAXFDS = 10000;
    int m_listener_socketfd; 
    int m_epoll_fd; 
    struct epoll_event* p_events;
    std::unique_ptr<peer_state_t[]> global_state;

private:
    //Change name
    fd_status_t on_peer_connected(int sockfd, const struct sockaddr_in* peer_addr,
            socklen_t peer_addr_len);
    fd_status_t on_peer_ready_recv(int sockfd);
    fd_status_t on_peer_ready_send(int sockfd);

};
