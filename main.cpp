//#include "server.hpp"
#include "db.hpp"

int main() {
    //Server server(9090);

    //server.run();
    Db db("10.89.60.33", "root", "cvscvs", "NhatAnh_Project");

    //db.getCategory(2);
    //db.getCategory("Soda");
    //db.getCategory("Drinks");
    //db.getCategory("Auto");

    //db.deleteCategory("Auto");
    //db.deleteCategory(9);
    //db.deleteCategory(10);
    std::string ref = db.getProduct(2);

    //db.getProductOfCategory("Soda");

    //db.addCategory(9, "Computer");
    //db.addCategory(10, "Computer", 1);
    //
    //db.addProduct(101, "Test", 19.2, "Testing", 2);
    return 0;
}
