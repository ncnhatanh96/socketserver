#include <mysql.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <memory>

MYSQL_RES *res;
MYSQL_ROW row;
#define GET_CATEGORY "select * from categories where categories.name = '%s';"
void getCategory(std::string category) {

    std::string query(
        "select * from categories where categories.name = "
    );

    query = query + "'" + category + "'" + ";";
    std::cout << "Debug: " << query << std::endl;

    if (mysql_query(conn, query.data())) {
        std::cout << "Unable to getCatgory" << std::endl;
    }

    //std::string query2(128);
    //snprintf(&query2[0], sizeof(query2), GET_CATEGORY, category.c_str());
    //std::cout << "Query2: " << query2 << "\n";
    res = mysql_store_result(conn);
    int num_of_field = mysql_num_fields(res);
    while(NULL != (row = mysql_fetch_row(res))) {
        std::cout << row[0] << " " << row[1] << "\n";
    }
    std::cout << std::endl;
    return;
}

char *server = "10.89.60.33";
char *user = "root";
char *password = "cvscvs"; /* set me first */
char *database = "NhatAnh_Project";

std::shared_ptr<MYSQL> connect() {
    MYSQL* conn = mysql_init(NULL);
    std::shared_ptr<MYSQL> _conn(conn);

   if (!mysql_real_connect(_conn.get(), server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(_conn.get()));
      exit(1);
   }

   return _conn;
}

int main() {

   std::shared_ptr<MYSQL> conn = connect();
   MYSQL* _conn = conn.get();
   //conn = mysql_init(NULL);
   std::cout << _conn << std::endl;

   ///* Connect to database */
   //if (!mysql_real_connect(conn, server,
   //      user, password, database, 0, NULL, 0)) {
   //   fprintf(stderr, "%s\n", mysql_error(conn));
   //   exit(1);
   //}

   /* send SQL query */
   if (mysql_query(_conn, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(_conn));
      exit(1);
   }

   res = mysql_use_result(_conn);

   //* output table name */
   printf("MySQL Tables in mysql database:\n");
   while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]);

   /* close connection */
   std::string category("Auto");
   //getCategory(category);
   mysql_free_result(res);
   mysql_close(_conn);

   std::cout << _conn << std::endl;
   return 0;
}
