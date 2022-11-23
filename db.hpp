#include <mysql.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <string>
#include "util.hpp"


class Db {
    private:
        std::string server;
        std::string user;
        std::string password;
        std::string database;

    private:
        MYSQL* connect();
        void close(MYSQL* conn);
        MYSQL_RES* query(std::string queries);
        void dumpSQLQuery(MYSQL_RES* res);

    public:

        Db(const std::string& server, const std::string& user,
            const std::string& password, const std::string& database);
        ~Db();

        void getCategory(std::string category);
        void getCategory(int id);

        void addCategory(std::string category);
        void addCategory(std::string category, int id);

        void deleteCategory(std::string category);
        void deleteCategory(int id);

        void getProductOfCategory(std::string category);
        void getProductOfCategory(int categoryID);

        void addProduct(std::string product, std::string category);

        void deleteProduct(std::string product);
};
