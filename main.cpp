//#include "server.hpp"
#include "db.hpp"

int main() {
    //Server server(9090);

    //server.run();
    Db db("10.89.60.33", "root", "cvscvs", "NhatAnh_Project");

    db.getCategory(1);
    return 0;
}
