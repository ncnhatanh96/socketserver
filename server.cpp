#include "server.hpp"

Server::Server(int portnum)
{

    if(-1 == portnum) {
        return;
    }
    m_listener_socketfd = listen_inet_socket(portnum);
    make_socket_non_blocking(m_listener_socketfd);

    m_epoll_fd = epoll_create1(0);
    if(m_epoll_fd < 0) {
        perror_die("epoll_create1");
    }

    struct epoll_event accept_event;
    accept_event.data.fd = m_listener_socketfd;
    accept_event.events = EPOLLIN;
    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listener_socketfd, &accept_event) < 0) {
        perror_die("epoll_ctl: EPOLL_CTL_ADD");
    }

    p_events = new epoll_event[MAXFDS];
    if(NULL == p_events) {
        perror_die("Unable to allocate memory for epoll_events");
    }
}

Server::~Server()
{
    return;
}

void Server::run()
{
    while(true)
    {
        int nready = epoll_wait(m_epoll_fd, p_events, MAXFDS, -1);
        for (int i = 0; i < nready; i++)
        {
            if(p_events[i].events & EPOLLERR)
            {
                perror_die("epoll_wait returned EPOLLERR");
            }

            if(m_listener_socketfd == p_events[i].data.fd)
            {
                struct sockaddr_in peer_addr;
                socklen_t peer_addr_len = sizeof(peer_addr);
                int new_sockfd = accept(m_listener_socketfd, (struct sockaddr*) &peer_addr,
                                        &peer_addr_len);
                if(new_sockfd < 0)
                {
                    if(eerno == EAGAIN || errnor == EWOULDBLOCK)
                    {
                        std::cout << "Accept returned EAGAIN | EWOULDBLOCK" << "\n";
                    }
                    else
                    {
                        //check
                        perror_die("accept");
                    }
                }
                else
                {
                    make_socket_non_blocking(new_sockfd);
                    if(new_sockfd >= MAXFDS)
                    {
                        die("socket fd (%d) >= MAXFDS (%d)", new_sockfd, MAXFDS);
                    }
                    //handle new client connection
                    fd_status_t status =
                        on_peer_connected(new_sockfd, &peer_addr, peer_addr_len);
                    struct epoll_event event = {0};
                    event.data.fd = new_sockfd;
                    if(status.want_read)
                }
            }
        }
    }
    return;
}
