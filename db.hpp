#include <mysql.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <string>
#include "util.hpp"


class db {
    private:
        std::string server;
        std::string user;
        std::string password;
        std::string database;

    public:
        db(std::string server, std::string user, std::string password, std::string database);
        ~db();

        std::shared_ptr<MYSQL> connect();
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
