#include "server.hpp"

Server::Server(int portnum)
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

    m_ConnectionStates = std::unique_ptr<ConnectionState[]>(new (std::nothrow) ConnectionState[MAXFDS]);
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
            event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
            break;

        case IOEvent::IO_NO_READ_WRITE:
            break;

        default:
            perror_die("Unknown action");
    }

    int res = 0;
    if (EPOLL_CTL_DEL == operation) {
        res = epoll_ctl(m_EpollFd, operation, connection_state->fd, nullptr);
        connection_state->reset();
    }
    else
        res = epoll_ctl(m_EpollFd, operation, connection_state->fd, &event);

    if (-1 == res) {
        perror_die("epoll_ctl error");
    }
    return;
}

void Server::Parser(std::string request) {
    std::string action(1, request[0]);
    char Product[32] = "\0";
    char Category[32] = "\0";
    float Price;
    char Desc[32];
    int ID = 0, CategoryID = 0;

    switch(atoi(action.c_str())) {

        case Action::AddCategory:

            break;

        case Action::AddProduct:
            sscanf(request.c_str(), "%*d_%d_%s_%f_%s_%d", 
                    &ID, Product, &Price, Desc, &CategoryID);
            std::cout << "ID: " << ID 
                    << " Price: " << Price
                    << " Desc: " << Desc
                    << " CategoryID: " << CategoryID << "\n";
            break;

        case Action::GetCategory:
            break;
        case Action::GetProduct:
            break;
        case Action::RemoveCategory:
            break;
        case Action::RemoveProduct:
            break;
        default:
            std::cout << "Unknown request from client\n";
            break;
    }
    return;
}
void Server::Read(std::shared_ptr<Db> db, ConnectionState* p_ConnectionState) {

    std::vector<char> buffer(MAX_BUF_LENGTH);
    std::string receive;

    auto bytes = recv(p_ConnectionState->fd, &buffer[0], buffer.size(), 0);

    if (0 == bytes) {
        p_ConnectionState->event = IO_NO_READ_WRITE;
        SetIOEvent(p_ConnectionState, EPOLL_CTL_DEL);
    } else if (bytes < 0) {
        if (EAGAIN == errno || EWOULDBLOCK == errno) {
            return;
        } else {
            perror_die("Receive error");
        }
    }
    receive.append(buffer.cbegin(), buffer.cend());
    Parser(receive);
    p_ConnectionState->event = IO_WRITE;
    return;
}

void Server::Write(ConnectionState* p_ConnectionState) {

    if (!p_ConnectionState) {
        return;
    }

    std::string buffer = "Writing..\nHehe\n";
    auto bytes = send(p_ConnectionState->fd, buffer.data(), buffer.length(), 0);
    if (bytes < buffer.length()) {
        std::cout << "Write error\n" << "\n";
    }

    p_ConnectionState->event = IO_READ;
    return;
}

void Server::WaitEvents(std::shared_ptr<Db> db) {

    std::cout << "WaitEvents...." << "\n";
    while(true) {
        struct epoll_event* Events = (struct epoll_event*)m_Events.get();
        auto eventsCount = epoll_wait(m_EpollFd, Events, MAXFDS, -1);

        if((eventsCount < 0 ) && errno != EINTR) {
            perror_die("epoll_wait error");
        }

        for (int i = 0; i < eventsCount; i++) {
            if (Events[i].data.ptr == this) {
                continue;
            }

            void* dataPtr = Events[i].data.ptr;
            ConnectionState* p_ConnectionState = (ConnectionState *) dataPtr;

            switch(p_ConnectionState->event) {

                case IOEvent::IO_READ:
                    std::cout << "Reading..." << "\n";
                    Read(db, p_ConnectionState);
                    SetIOEvent(p_ConnectionState, EPOLL_CTL_MOD);
                    break;

                case IOEvent::IO_WRITE:
                    std::cout << "Writing..." << "\n";
                    Write(p_ConnectionState);
                    SetIOEvent(p_ConnectionState, EPOLL_CTL_MOD);
                    break;

                default:
                    perror_die("Error handling fd");
                    break;
            }
        }
    }
    return;
}

void Server::Run()
{
    auto db = std::shared_ptr<Db>(new Db("10.89.60.33", "root", "cvscvs", "NhatAnh_Project"));
    std::thread polling(&Server::WaitEvents, this, db);
    polling.detach();

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
        m_ConnectionStates[newsockfd].fd = newsockfd;
        SetIOEvent(&m_ConnectionStates[newsockfd], EPOLL_CTL_ADD);
    }
    return;
}
