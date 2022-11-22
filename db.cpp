#include "db.hpp"
#define GET_CATEGORY "select * from categories where categories.name = '%s'"

db::db(std::string server, std::string user, std::string password) {
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user,
                password, database, 0, NULL, 0)) {

        perror_die("Unable to init mysql connection");
    }
}

db::~db() {
   mysql_close(conn);
}

void db::getCategory(std::string category) {

    std::string query(
        "select * from categories where categories.name = "
    );

    query = query + "'" + category + "'";
    std::cout << "Debug: " << query << std::endl;

    std::string query1;
    snprintf(query1.data(), GET_CATEGO)
    if (mysql_query(conn, query.data())) {
        std::cout << "Unable to getCatgory" << std::endl;
    }

    MYSQL_RES* res = mysql_use_result(conn);
    MYSQL_ROW row;
    while(NULL != (row = mysql_fetch_row(res))) {
        std::cout << row << " \n";
    }
    std::cout << std::endl;
    return;
}
void db::getCategory(int id) {
    return;
}

void db::addCategory(std::string category) {
    return;
}

void db::addCategory(std::string category, int id) {

}

void db::deleteCategory(std::string category) {
    return;
}
void db::deleteCategory(int id) {
    return;
}

void db::getProductOfCategory(std::string category) {
    return;
}

void db::getProductOfCategory(int categoryID) {
    return;
}

void db::addProduct(std::string product, std::string category) {
    return;
}

void db::deleteProduct(std::string product) {
    return;
}
