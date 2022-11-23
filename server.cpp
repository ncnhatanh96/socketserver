#include "server.hpp"

Server::Server(int portnum)
        :db("10.89.60.33", "root", "cvscvs", "NhatAnh_Project")
{
    if (-1 == portnum) {
        return;
    }
    m_ListenSocket = listen_inet_socket(portnum);

    m_EpollFd = epoll_create1(0);
    if (m_EpollFd < 0) {
        perror_die("epoll_create1");
    }

    m_Events = std::unique_ptr<epoll_event[]>(new (std::nothrow) epoll_event[MAXFDS]);
    if (!m_Events) {
        perror_die("Unable to allocate memory for epoll_events");
    }

    m_ConnectionStates= std::unique_ptr<ConnectionState[]>(new (std::nothrow) ConnectionState[MAXFDS]);
    if (!m_Events) {
        perror_die("Unable to allocate memory for epoll_events");
    }

}

Server::~Server() {
    return;
}

void Server::SetIOEvent(ConnectionState* connection_state, int operation) {

    struct epoll_event event;
    std::memset(&event, 0, sizeof(struct epoll_event));
    event.data.ptr = connection_state;

    switch(connection_state->event) {

        case IOEvent::IO_READ:
            event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
            break;

        case IOEvent::IO_WRITE:
            break;

        default:
            perror_die("Unknown action");
    }
    auto res = epoll_ctl(m_EpollFd, operation, connection_state->fd, &event);
    if (-1 == res) {
        perror_die("epoll_ctl error");
    }
    return;
}

//void read(Action* action) {
//
//    if (nullptr == action) {
//        std::cout << "NULL Ptr" << "\n";
//    }
//
//    uint8_t buf[1024];
//    auto nbytes = recv(action->getFd(), buf, sizeof(buf), 0);
//    std::cout << "nbytes: " << nbytes << "\n";
//    if (nbytes <= 0) {
//        std::cout << "Closing\n";
//        action->setIOEventType(Action::IOEvent_Type::IO_EVENT_NO_RW);
//        return;
//    }
//
//    std::cout << "Received message: " << buf << "\n";
//    action->setIOEventType(Action::IOEvent_Type::IO_EVENT_W);
//    return;
//}
//
//void write(Action* action) {
//
//    std::string buff("Default message\nhahaha\nhehehe");
//    auto nbytes = send(action->getFd(), buff.c_str(), 32, 0);
//
//    if(-1 == nbytes) {
//        std::cout << "Error sending message" << "\n";
//    } else if (nbytes < 15) {
//        std::cout << "Keep sending message" << "\n";
//    }
//    action->setIOEventType(Action::IOEvent_Type::IO_EVENT_R);
//    return;
//}

//void Server::WaitEvents() {
//
//    struct epoll_event* Events = (struct epoll_event*)m_Events.get();
//    auto eventsCount = epoll_wait(m_EpollFd, Events, MAXFDS, -1);
//
//    if((eventsCount < 0 ) && errno != EINTR) {
//        perror_die("epoll_wait error");
//    }
//
//    for(int i = 0; i < eventsCount; i++) {
//        if (Events[i].data.ptr == this) {
//            newConnection();
//            continue;
//        }
//
//        void* dataPtr = Events[i].data.ptr;
//        Action* action = (Action*) dataPtr;
//
//        switch(action->getIOEventType()) {
//
//            case Action::IOEvent_Type::IO_EVENT_R:
//                std::cout << "Reading..." << "\n";
//                read(action);
//                setIOEvent(action, EPOLL_CTL_MOD);
//                break;
//
//            case Action::IOEvent_Type::IO_EVENT_W:
//                std::cout << "Writing..." << "\n";
//                write(action);
//                setIOEvent(action, EPOLL_CTL_MOD);
//                break;
//
//            case Action::IOEvent_Type::IO_EVENT_NO_RW:
//                std::cout << "No read write" << "\n";
//                close(action->getFd());
//                setIOEvent(action, EPOLL_CTL_DEL);
//                break;
//
//            default:
//                perror_die("Error handling fd");
//                break;
//        }
//    }
//    return;
//}
//
void Server::Run()
{
    while(true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int newsockfd = accept(m_ListenSocket, (struct sockaddr*)& client_addr,
                                &client_addr_len);

        if (newsockfd < 0) {
            if (EAGAIN == errno || EWOULDBLOCK == errno) {
                std::cout << "Accept returned EAGAIN or EWOULDBLOCK" << "\n";
            } else {
                perror_die("Accept()");
            }
        } else if (newsockfd >= MAXFDS) {
            perror_die("sockfd greater than MAXFDS");
        }

        make_socket_non_blocking(newsockfd);
        m_ConnectionStates[newsockfd].reset();
        SetIOEvent(&m_ConnectionStates[newsockfd], EPOLL_CTL_ADD);
    }
    return;
}
