#include "db.hpp"

#define GET_CATEGORY_BY_NAME    "select category_id, name, parent_id "\
                                "from (select * from categories order by parent_id, category_id) products_sorted, "\
	                            "(select @pv := (select category_id from categories where name = '%s')) initialisation "\
                                "where find_in_set(parent_id, @pv) AND length(@pv := concat(@pv, ',', category_id));"

#define GET_CATEGORY_BY_ID      "select category_id, name, parent_id "\
                                "from (select * from categories order by parent_id, category_id) products_sorted, "\
	                            "(select @pv := '%d') initialisation "\
                                "where find_in_set(parent_id, @pv) AND length(@pv := concat(@pv, ',', category_id));"

std::string constructQuery(const char* query, int param) {
    char buf[1024];

    auto size = snprintf(buf, sizeof(buf), query, param);

    return std::string(buf, buf + size - 1);
}

Db::Db(const std::string& server, const std::string& user,
        const std::string& password, const std::string& database)
    :server(server), password(password), user(user), database(database) {
}

Db::~Db() {
}

MYSQL* Db::connect() {

	MYSQL* conn = NULL;

    conn = mysql_init(NULL);

    if (conn == NULL) {
		std::cout << "Cannot create SQL connection" << "\n";
        return NULL;
    } else {
        if (!mysql_real_connect(conn, server.c_str(), user.c_str(),
            password.c_str(), database.c_str(), 0, NULL, 0))
        {
			std::cout << "Cannot connection to database" << "\n";
            mysql_close(conn);
            return NULL;
        } else {
            bool reconnect = 1;
            mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
        }
    }
    return conn;
}

void Db::close(MYSQL* conn) {

	if(nullptr != conn)
		mysql_close(conn);

    return;
}

MYSQL_RES* Db::query(std::string queries) {

    MYSQL* conn = connect();

    if (mysql_query(conn, queries.data())) {
        std::cout << "Query error" << std::endl;
        close(conn);
        return nullptr;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if(res)
        dumpSQLQuery(res);
    close(conn);

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

void Db::getCategory(std::string category) {

    char _queries[1024];
    snprintf(_queries, sizeof(_queries), GET_CATEGORY_BY_NAME, category.data());
    printf("%s\n", _queries);
    //queries = queries + "'" + category + "'";

    query(_queries);

    return;
}
void Db::getCategory(int id) {

    char _queries[1024];
    snprintf(_queries, sizeof(_queries), GET_CATEGORY_BY_ID, id);
    printf("%s\n", _queries);

    std::string test = constructQuery(GET_CATEGORY_BY_ID, id);
    std::cout << test << "\n";
//    std::string queries (
//        "select * from categories where categories.id = "
//    );
//
//    queries = queries + "'" + std::to_string(id) + "'";
//
    query(_queries);

    return;
}

void Db::addCategory(std::string category) {

    std::string queries (
        "insert into categories(name) values "
    );

    queries = queries + "('" + category + "');";
    std::cout << "queries: " << queries << "\n";
    query(queries);

    return;
}

void Db::addCategory(std::string category, int id) {

    std::string queries (
        "insert into categories(name, id) values "
    );

    queries = queries
                + "('" + category + "',"
                + "'" + std::to_string(id) + "');";
    std::cout << "queries: " << queries << "\n";
    query(queries);
    return;
}

void Db::deleteCategory(std::string category) {

    std::string queries (
        "delete categories, products, properties from categories \
        left join products on products.category_id = categories.id \
        left join properties on properties.product_id = products.id \
        where categories.name = "
    );

    queries = queries + "'" + category + "'" + ";";
    std::cout << queries << "\n";
    query(queries);
    return;
}
void Db::deleteCategory(int id) {

    std::string queries (
        "delete categories, products, properties from categories \
        left join products on products.category_id = categories.id \
        left join properties on properties.product_id = products.id \
        where categories.name = "
    );

    queries = queries + "'" + std::to_string(id)+ "'" + ";";
    std::cout << queries << "\n";
    query(queries);
    return;
}

void Db::getProductOfCategory(std::string category) {

    std::string queries (
        "select * from products where categories.name = "
    );

    queries = queries + "'" + category + "'";

    query(queries);
    return;
}

void Db::getProductOfCategory(int categoryID) {
    return;
}

void Db::addProduct(std::string product, std::string category) {
    return;
}

void Db::deleteProduct(std::string product) {
    return;
}
