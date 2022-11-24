#include "server.hpp"

int main() {
    Server server(9090);

    server.Run();
    return 0;
}
