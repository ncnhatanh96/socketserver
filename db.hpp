#ifndef _DB_HPP_
#define _DB_HPP_ 
#include <mysql.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include "util.hpp"

#define GET_CATEGORY_BY_NAME    "select category_id, name, parent_id "\
                                "from (select * from categories order by parent_id, category_id) products_sorted, "\
	                            "(select @pv := (select category_id from categories where name = '%s')) initialisation "\
                                "where find_in_set(parent_id, @pv) AND length(@pv := concat(@pv, ',', category_id));"

#define GET_CATEGORY_BY_ID      "select category_id, name, parent_id "\
                                "from (select * from categories order by parent_id, category_id) products_sorted, "\
	                            "(select @pv := '%d') initialisation "\
                                "where find_in_set(parent_id, @pv) AND length(@pv := concat(@pv, ',', category_id));"
class Db {
private:
    std::string server;
    std::string user;
    std::string password;
    std::string database;
	MYSQL* conn = NULL;

private:
    MYSQL_RES* query(std::string queries);
    void dumpSQLQuery(MYSQL_RES* res);
    std::string toString(MYSQL_RES* res);

public:

    Db(const std::string& server, const std::string& user,
        const std::string& password, const std::string& database);
    ~Db();

    const std::string getCategory(std::string category);
    const std::string getCategory(int id);
    const std::string getProduct(std::string category);
    const std::string getProduct(int categoryID);

    void addCategory(int id, std::string new_name, std::string parent_name);
    void addCategory(int id, std::string new_name, int parent_id);
    void addCategory(int id, std::string new_name);
    void addProduct(int id, std::string name, float price,
                    std::string desc, int category_id);
    void addProduct(int id, std::string name, float price,
                    std::string desc, std::string category_name);

    void deleteCategory(std::string category);
    void deleteCategory(int id);
    void deleteProduct(std::string product);
    void deleteProduct(int id);
};

#endif /*_DB_HPP_*/
