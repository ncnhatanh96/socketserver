#include "server.hpp"

Server::Server(int portnum, int threads):m_Threads(threads)
{
    std::cout << "Initializing server...\n";
    if (-1 == portnum) {
        return;
    }
    m_ListenSocket = Start_ListenSocket(portnum);

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

        case IOEvent::IO_CLOSED:
            return;

        default:
            perror_die("Unknown action");
    }

    int res = 0;
    if (EPOLL_CTL_DEL == operation) {
        std::cout << "Close socket...\n";
        res = epoll_ctl(m_EpollFd, operation, connection_state->fd, nullptr);
        close(connection_state->fd);
        connection_state->reset();
    }
    else
        res = epoll_ctl(m_EpollFd, operation, connection_state->fd, &event);

    if (-1 == res) {
        std::cout << "Ignore socket error...\n";
    }
    return;
}

void Server::Handler(std::shared_ptr<Db> db, std::string request, ConnectionState* p_ConnectionState) {
    std::cout << "Handling request from client...\n";
    std::string action(1, request[0]);
    char Product[32]{};
    char Category[32]{};
    float Price;
    char Desc[32];
    int ID = 0, CategoryID = 0, ParentID = -1;

    switch(atoi(action.c_str())) {

        case Action::AddCategory:
            sscanf(request.c_str(), "%*d_%d_%[^_]_%d_", &CategoryID, Category, &ParentID);

            if (-1 == ParentID)
                db->addCategory(CategoryID, Category);
            else
                db->addCategory(CategoryID, Category, ParentID);
            break;

        case Action::AddProduct:
            sscanf(request.c_str(), "%*d_%d_%[^_]_%f_%[^_]_%[^_]", &ID, Product, &Price, Desc, Category);
            db->addProduct(ID, Product, Price, Desc, Category);
            break;

        case Action::GetCategory:
            sscanf(request.c_str(), "%*d_%[^_]_", Category);
            std::cout << "Category: " << Category << "\n";
            p_ConnectionState->buff = db->getCategory(Category);
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
        default:
            break;
    }
    return;
}
void Server::Read(std::shared_ptr<Db> db, ConnectionState* p_ConnectionState) {

    std::vector<char> buffer(MAX_BUF_LENGTH);
    std::string receive;

    auto bytes = recv(p_ConnectionState->fd, &buffer[0], buffer.size(), 0);
    if (bytes <= 0) {
        p_ConnectionState->event = IO_NO_READ_WRITE;
        SetIOEvent(p_ConnectionState, EPOLL_CTL_DEL);
        return;
    }
    receive.append(buffer.cbegin(), buffer.cend());
    Handler(db, receive, p_ConnectionState);
    p_ConnectionState->event = IO_WRITE;
    return;
}

void Server::Write(ConnectionState* p_ConnectionState) {

    if (!p_ConnectionState) {
        perror_die("Write: nullptr");
    }

    const char* buff_ptr = p_ConnectionState->buff.data();
    std::size_t buff_size = p_ConnectionState->buff.length();
    int bytes = 0;
    while (buff_size > bytes) {
        auto bytes = send(p_ConnectionState->fd, p_ConnectionState->buff.data(),
                            p_ConnectionState->buff.length(), 0);
        if (-1 == bytes) {
            if (EAGAIN == errno || errno == EWOULDBLOCK)
                return;
            else
                perror_die("Send error");
        }

        buff_ptr += bytes;
        buff_size -= bytes;
    }
    p_ConnectionState->buff.clear();
    p_ConnectionState->event = IO_READ;
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
        std::cout << "Accept new connection...\n";
        ReportClientConnection(&client_addr, client_addr_len);
        Set_NONBlocking_Socket(newsockfd);

        m_ConnectionStates[newsockfd].reset();
        m_ConnectionStates[newsockfd].fd = newsockfd;
        m_ConnectionStates[newsockfd].event = IOEvent::IO_READ;
        SetIOEvent(&m_ConnectionStates[newsockfd], EPOLL_CTL_ADD);
    }
    return;
}

void Server::WaitIOEvents(std::shared_ptr<Db> db) {
    std::cout << "Polling thread is running...\n";
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
                    Read(db, p_ConnectionState);
                    SetIOEvent(p_ConnectionState, EPOLL_CTL_MOD);
                    break;

                case IOEvent::IO_WRITE:
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

