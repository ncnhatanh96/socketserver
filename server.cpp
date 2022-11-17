#include "server.hpp"

Server::Server(int portnum)
{

    if (-1 == portnum) {
        return;
    }
    m_listener_socketfd = listen_inet_socket(portnum);
    make_socket_non_blocking(m_listener_socketfd);

    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd < 0) {
        perror_die("epoll_create1");
    }

    struct epoll_event accept_event;
    accept_event.data.fd = m_listener_socketfd;
    accept_event.events = EPOLLIN;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listener_socketfd, &accept_event) < 0) {
        perror_die("epoll_ctl: EPOLL_CTL_ADD");
    }

    p_events = new epoll_event[MAXFDS];
    if (!p_events) {
        perror_die("Unable to allocate memory for epoll_events");
    }

    global_state = std::unique_ptr<peer_state_t[]>(new (std::nothrow) peer_state_t[MAXFDS]);
    if (!global_state) {
        perror_die("Unable to allocate memory for peer_state_t");
    }
}

Server::~Server()
{
    delete p_events;
    delete global_state.get();
    return;
}

void Server::run()
{
    while(true)
    {
                std::cout << "Line: " <<  __LINE__ << "\n";
        int nready = epoll_wait(m_epoll_fd, p_events, MAXFDS, -1);
                std::cout << "Line: " <<  __LINE__ << "\n";
        for (int i = 0; i < nready; i++) {
                std::cout << "Line: " <<  __LINE__ << "\n";
            if (p_events[i].events & EPOLLERR) {

                perror_die("epoll_wait returned EPOLLERR");
            }

            if (m_listener_socketfd == p_events[i].data.fd) {

                std::cout << "Line: " <<  __LINE__ << "\n";
                struct sockaddr_in peer_addr;
                socklen_t peer_addr_len = sizeof(peer_addr);

                int new_sockfd = accept(m_listener_socketfd, (struct sockaddr*) &peer_addr,
                                        &peer_addr_len);
                if (new_sockfd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        std::cout << "Accept returned EAGAIN | EWOULDBLOCK" << "\n";
                    } else {
                        //check
                        perror_die("accept");
                    }
                } else {
                    /*PUT TO HANDLER CLASS*/
                std::cout << "Line: " <<  __LINE__ << "\n";
                    make_socket_non_blocking(new_sockfd);
                    if (new_sockfd >= MAXFDS) {
                        die("socket fd (%d) >= MAXFDS (%d)", new_sockfd, MAXFDS);
                    }

                    //handle new client connection
                    fd_status_t status =
                        on_peer_connected(new_sockfd, &peer_addr, peer_addr_len);
                    struct epoll_event event = {0};
                    event.data.fd = new_sockfd;

                    if (status.want_read) {
                        event.events |= EPOLLIN;
                    }

                    if (status.want_write) {
                        event.events |= EPOLLOUT;
                    }

                    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, new_sockfd, &event) < 0) {
                        perror_die("epoll_ctl EPOLL_CTL_ADD");
                    }
                }
            } else {
                /*IO_READ_HANDLER - RECEIVE*/
                std::cout << "Line: " <<  __LINE__ << "\n";
                if (p_events[i].events & EPOLLIN) {
                    std::cout << "Line: " <<  __LINE__ << "\n";
                    int fd = p_events[i].data.fd;
                    fd_status_t status = on_peer_ready_recv(fd);
                    struct epoll_event event = {0};
                    event.data.fd = fd;

                    if (status.want_read) {
                        event.events |= EPOLLIN;
                    }

                    if (status.want_write) {
                        event.events |= EPOLLOUT;
                    }

                    if (0 == event.events) {
                        printf("socket %d closing\n", fd);
                        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
                            perror_die("epoll_ctl EPOLL_CTL_ADD");
                        }
                        close(fd);
                    } else if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
                        perror_die("epoll_ctl EPOLL_CTL_MOD");
                    }
                } else if (p_events[i].events & EPOLLOUT) {
                    /*IO_WRITE_HANDLER - */
                    int fd = p_events[i].data.fd;
                    fd_status_t status = on_peer_ready_send(fd);
                    struct epoll_event event = {0};
                    event.data.fd = fd;

                    /*Wrap to handler or rcoutine get action*/
                    if (status.want_read) {
                        event.events |= EPOLLIN;
                    }

                    if (status.want_write) {
                        event.events |= EPOLLOUT;
                    }

                    /*Wrap close socket*/
                    if (0 == event.events) {
                        printf("socket %d closing\n", fd);
                        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
                            perror_die("epoll_ctl EPOLL_CTL_DEL");
                        }
                        close(fd);
                    } else if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
                        perror_die("epoll_ctl EPOLL_CTL_MOD");
                    }
                }
            }
        }
    }
    return;
}



/*Write Handler*/
fd_status_t Server::on_peer_connected(int sockfd, const struct sockaddr_in* peer_addr,
                                socklen_t peer_addr_len) {
    assert(sockfd < MAXFDS);
    report_peer_connected(peer_addr, peer_addr_len);

    //Intialize state to send back
    peer_state_t* peerstate = &global_state.get()[sockfd];
    peerstate->state = INITIAL_ACK;

    return fd_status_R;
}

fd_status_t Server::on_peer_ready_recv(int sockfd) {
    assert(sockfd < MAXFDS);
    peer_state_t* peerstate = &global_state.get()[sockfd];

    /*GET_BUFFER - Need to parse request from client within this*/
    uint8_t buf[1024];

    int nbytes = recv(sockfd, buf, sizeof(buf), 0);
    if (0 == nbytes) {
        return fd_status_NORW;
    } else if (nbytes < 0) {
        if (EAGAIN == errno || errno == EWOULDBLOCK) {
            return fd_status_R;
        } else {
        perror_die("recv");
        }
    }
    std::cout << "Buf: " << buf << "\n";

    return (fd_status_t) {
                .want_read = false,
                .want_write = true,
            };
}

fd_status_t Server::on_peer_ready_send(int sockfd) {
    return fd_status_R;

    assert(sockfd < MAXFDS);
    peer_state_t* peerstate = &global_state.get()[sockfd];

    std::memset(&peerstate->sendbuf, SENDBUF_SIZE);
    int nsent = send(sockfd, &peerstate->sendbuf, sendlen, 0);
    if (nsent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return fd_status_W;
        } else {
          perror_die("send");
        }
    }
}
