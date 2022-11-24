#include "server.hpp"

Server::Server(int portnum, int threads):m_Threads(threads)
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

    for (int i = 0; i < m_Threads; i++) {
        auto db = std::shared_ptr<Db>(new Db("10.89.60.33", "root", "cvscvs", "NhatAnh_Project"));
        std::thread polling(&Server::WaitIOEvents, this, db);
        polling.detach();
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

void Server::Handler(std::shared_ptr<Db> db, std::string request, ConnectionState* p_ConnectionState) {
    std::string action(1, request[0]);
    char Product[32]{};
    char Category[32]{};
    float Price;
    char Desc[32];
    int ID = 0, CategoryID = 0, ParentID = -1;

    switch(atoi(action.c_str())) {

        case Action::AddCategory:
            sscanf(request.c_str(), "%*d_%d_%[^_]_%d_", &CategoryID, Category, &ParentID);
            std::cout << "CategoryID: " << CategoryID
                    << " Category: " << Category
                    << " ParentID: " << ParentID << "\n";
            if (-1 == ParentID)
                db->addCategory(CategoryID, Category);
            else
                db->addCategory(CategoryID, Category, ParentID);
            break;

        case Action::AddProduct:
            sscanf(request.c_str(), "%*d_%d_%[^_]_%f_%[^_]_%d", &ID, Product, &Price, Desc, &CategoryID);
            std::cout << "ID: " << ID
                    << " Product: " << Product
                    << " Price: " << Price
                    << " Desc: " << Desc
                    << " CateID: " << CategoryID << "\n";
            db->addProduct(ID, Product, Price, Desc, CategoryID);
            break;

        case Action::GetCategory:
            sscanf(request.c_str(), "%*d_%[^_]_", Category);
            p_ConnectionState->buff = db->getCategory(Category);
            std::cout << "Buff: " << p_ConnectionState->buff << "\n";
            std::cout << "Fd: " << p_ConnectionState->fd << "\n";

            break;

        case Action::GetProduct:
            sscanf(request.c_str(), "%*d_%[^_]_", Category);
            p_ConnectionState->buff = db->getProduct(Category);
            break;

        case Action::RemoveCategory:
            sscanf(request.c_str(), "%*d_%[^_]_", Category);
            db->deleteCategory(Category);
            break;

        case Action::RemoveProduct:
            sscanf(request.c_str(), "%*d_%[^_]_", Product);
            db->deleteProduct(Product);
            break;
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
    Handler(db, receive, p_ConnectionState);
    p_ConnectionState->event = IO_WRITE;
    return;
}

void Server::Write(ConnectionState* p_ConnectionState) {

    if (!p_ConnectionState) {
        std::cout << "Access NULLPtr\n";
        return;
    }
    std::cout << "Sending..." << "\n";
    std::cout << "Buff: " << p_ConnectionState->buff <<  "\n";
    std::cout << "Fd: " << p_ConnectionState->fd << "\n";
    auto bytes = send(p_ConnectionState->fd,
            p_ConnectionState->buff.data(), p_ConnectionState->buff.length(), 0);
    if (bytes < p_ConnectionState->buff.length()) {
        std::cout << "Write error\n" << "\n";
    }
    std::cout << "Done\n" << "\n";
    p_ConnectionState->buff.clear();
    p_ConnectionState->event = IO_READ;
    return;
}

void Server::WaitIOEvents(std::shared_ptr<Db> db) {

    std::cout << "WaitIOEvents...." << "\n";
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
