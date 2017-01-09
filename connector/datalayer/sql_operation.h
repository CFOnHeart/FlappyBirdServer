#include <mysql/mysql.h>
#include "../config/configure.h"
#define MYSQL_INI "/home/ganjun/Desktop/version4.0/connector/config.ini/mysql.ini"
#define clear(a) memset(a,0,sizeof(a))
namespace mysql{
    class GJMysql{
    public:
        MYSQL *g_conn;
        MYSQL_RES *g_res;
        MYSQL_ROW g_row;
        //通过配置文件mysql.ini读取获得数据库连接信息
        char MYSQLIP[20] , USER[20] , PASSWORD[20] , DATABASE[20];

        Config::Configure config;
        
        GJMysql();
        
        ~GJMysql();
        
        void init_config();
        
        void print_mysql_error(const char *msg);
        
        int32_t init_mysql();
        
        int32_t query_sql(const char *sql);
        
        int32_t executesql(const char *sql);
        
        bool isExistUser(const char *username);
        
        bool isValidUser(const char *username , const char *password);
    
        bool addUserInfo(int32_t uid , const char *username , const char *password , int32_t score);
        
        int32_t getBestScore(const char *username , const char *password);
        
        int32_t getBestScore(const char *username);
        
        int32_t getBestScore(int32_t uid);
         
        bool updateScore(const char *username , int32_t score);
        
        bool updateScore(int32_t uid , int32_t score);

        void close();
    };
};
