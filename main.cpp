#include "server.hpp"


int main() {
    Server server(9090);

    server.run();

    return 0;
}
