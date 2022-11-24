#include "db.hpp"

Db::Db(const std::string& server, const std::string& user,
        const std::string& password, const std::string& database)
    :server(server), password(password), user(user), database(database) {

    conn = mysql_init(NULL);

    if (conn == NULL) {
		std::cout << "Cannot create SQL connection" << "\n";
        return;
    } else {
        if (!mysql_real_connect(conn, server.c_str(), user.c_str(),
            password.c_str(), database.c_str(), 0, NULL, 0))
        {
			std::cout << "Cannot connection to database" << "\n";
            mysql_close(conn);
            return;
        } else {
            bool reconnect = 1;
            mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
        }
    }
}

Db::~Db() {
	mysql_close(conn);
}

MYSQL_RES* Db::query(std::string queries) {

    if (mysql_query(conn, queries.data())) {
        std::cout << "Query error" << std::endl;
        return nullptr;
    }

    MYSQL_RES* res = mysql_store_result(conn);

    return res;
}

void Db::dumpSQLQuery(MYSQL_RES* res) {

    MYSQL_ROW row;
    int num_of_fields = mysql_num_fields(res);

    while (NULL != (row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_of_fields; i++) {
            std::cout << " " << row[i];
        }
        std::cout << "\n";
    }
    return;
}

std::string Db::toString(MYSQL_RES* res) {

    MYSQL_ROW row;
    int num_of_fields = mysql_num_fields(res);
    std::string ret;
    std::ostringstream _ret;

    if (!res) {
        return ret;
    }

    while (NULL != (row = mysql_fetch_row(res))) {
        for (int i = 0; i < num_of_fields; i++) {
            _ret << row[i] << " ";
        }
        _ret << "\n";
    }
    ret = _ret.str();
    return ret;
}

void Db::addCategory(int id, std::string new_name, std::string parent_name) {

    std::string queries (
        "insert into categories(category_id, name, parent_id) values "
    );

    queries = queries
                + "(" + std::to_string(id) + ","
                + "'" + new_name+ "',"
                + "(" + "select category_id from categories where name = '" + parent_name + "));";

    query(queries);
    return;
}

void Db::addCategory(int id, std::string new_name) {

    std::string queries (
        "insert into categories(category_id, name) values "
    );

    queries = queries
                + "(" + std::to_string(id) + ","
                + "'" + new_name+ "'" +  ");";

    query(queries);
    return;
}

void Db::addCategory(int id, std::string new_name, int parent_id) {

    std::string queries (
        "insert into categories(category_id, name, parent_id) values "
    );

    queries = queries
                + "(" + std::to_string(id) + ","
                + "'" + new_name+ "',"
                + std::to_string(parent_id) + ");";

    query(queries);
    return;
}

void Db::addProduct(int id, std::string name, float price,
                        std::string desc, int category_id) {

    std::string queries (
        "insert into products values"
    );

    queries = queries
                + "(" + std::to_string(id) + ","
                + "'" + name + "'" + ","
                + std::to_string(price) + ","
                + "'" + desc + "'" + ","
                + std::to_string(category_id) + ");";
    query(queries);
    return;
}

void Db::addProduct(int id, std::string name, float price,
                        std::string desc, std::string category_name) {

    std::string queries (
        "insert into products values"
    );

    queries = queries
                + "(" + std::to_string(id) + ","
                + "'" + name + "'" + ","
                + std::to_string(price) + ","
                + "'" + desc + "'" + ","
                + "(select category_id from categories where name = '"
                + category_name + "'));";
    query(queries);
    return;
}

void Db::deleteCategory(std::string category) {

    std::string queries (
        "delete categories from categories where name = "
    );

    queries = queries + "'" + category + "'" + ";";
    query(queries);
    return;
}

void Db::deleteCategory(int id) {

    std::string queries (
        "delete categories from categories where category_id = "
    );

    queries = queries +  std::to_string(id) + ";";
    query(queries);
    return;
}

void Db::deleteProduct(std::string product) {

    std::string queries (
        "delete from products where name = "
    );

    queries = queries + "'" + product + "';";
    query(queries);
    return;
}

void Db::deleteProduct(int id) {

    std::string queries (
        "delete from products where id = "
    );

    queries = queries + std::to_string(id) + ";";
    query(queries);
    return;
}

const std::string Db::getCategory(std::string category) {

    std::string queries (
        "select category_id, name from categories \
        where parent_id = (select category_id from categories where name = "
    );

    queries = queries + "'" + category + "');";
    std::cout << "queries: " << queries << "\n";
    MYSQL_RES* res = query(queries);
    return toString(res);
}

const std::string Db::getCategory(int id) {

    std::string queries (
        "select category_id, name from categories where parent_id = "
    );

    queries = queries + std::to_string(id) + ";";
    MYSQL_RES* res = query(queries);
    return toString(res);
}

const std::string Db::getProduct(std::string category) {

    std::string queries (
        "select * from products \
        where category_id = (select category_id from categories where name = "
    );

    queries = queries + "'" + category + "');";
    MYSQL_RES* res = query(queries);
    return toString(res);;
}

const std::string Db::getProduct(int categoryID) {

    std::string queries (
        "select * from products  where category_id = "
    );

    queries = queries + std::to_string(categoryID) + ";";
    query(queries);
    MYSQL_RES* res = query(queries);
    return toString(res);
}
