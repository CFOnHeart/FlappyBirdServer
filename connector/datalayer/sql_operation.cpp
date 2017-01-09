/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "sql_operation.h"

mysql::GJMysql::GJMysql(){
    memset(MYSQLIP , 0 , sizeof(MYSQLIP));
    memset(USER , 0 , sizeof(USER));
    memset(PASSWORD , 0 , sizeof(PASSWORD));
    memset(DATABASE , 0 , sizeof(DATABASE));
}

mysql::GJMysql::~GJMysql(){
    
}

void mysql::GJMysql::init_config(){
    config = Config::Configure(MYSQL_INI);
    config.analyse();
    clear(MYSQLIP);
    clear(USER);
    clear(PASSWORD);
    clear(DATABASE);
    strcpy(MYSQLIP , config["MYSQLIP"].c_str());
    strcpy(USER , config["USER"].c_str());
    strcpy(PASSWORD , config["PASSWORD"].c_str());
    strcpy(DATABASE , config["DATABASE"].c_str());
    config.close();
}

void mysql::GJMysql::print_mysql_error(const char *msg){
    if(msg)
        printf("%s: %s\n" , msg , mysql_error(g_conn));
    else 
        puts(mysql_error(g_conn));
}
int mysql::GJMysql::init_mysql(){
    g_conn = mysql_init(NULL);

    if(!mysql_real_connect(g_conn , MYSQLIP , USER , PASSWORD , DATABASE , 0 , NULL , 0))
        return -1;
    return 0;
}

int mysql::GJMysql::query_sql(const char *sql){
    if(mysql_real_query(g_conn , sql , strlen(sql)))
        return -1;
    return 0;
}
//执行sql语句 
int mysql::GJMysql::executesql(const char *sql){
   if(mysql_real_query(g_conn , sql , strlen(sql)))
       return -1;
   return 0;
}
//判断用户是否存在
bool mysql::GJMysql::isExistUser(const char *username){
    char sql[1024] = "select * from UserInfo where username=";
    sprintf(sql , "select * from UserInfo where username='%s'" , username);
    if(executesql(sql))
        print_mysql_error(NULL);
    g_res = mysql_store_result(g_conn);
    bool flag=  mysql_num_rows(g_res)>0;

    mysql_free_result(g_res);
    return flag;
}
//判断用户名密码是否正确用于登录
bool mysql::GJMysql::isValidUser(const char *username , const char *password){
    char sql[1024] = {'\0'};
    sprintf(sql , "select username,password from UserInfo where username='%s'" , username);
    printf("sql : %s\n" , sql);
    if(executesql(sql))
        print_mysql_error(NULL);
    bool flag = false;

    g_res = mysql_store_result(g_conn);
    if(mysql_num_rows(g_res) == 0){
        mysql_free_result(g_res);
        return false;
    }
    while(g_row = mysql_fetch_row(g_res)){
        if(strcmp(username , g_row[0]) == 0 && strcmp(password , g_row[1]) == 0)
            flag = true;
    }
    mysql_free_result(g_res);
    return flag;
}
//用于添加用户信息，username是primary key，所以关键字一样无法添加并返回false
bool mysql::GJMysql::addUserInfo(int32_t uid , const char *username , const char *password , int32_t score){
    char sql[1024] = {'\0'};
    sprintf(sql , "insert into UserInfo values(%d,'%s','%s',%d)" ,uid , username,password,score);
    if(executesql(sql))
    {    
        print_mysql_error(NULL);
        return false;
    }
    return (unsigned long)mysql_affected_rows(g_conn) == 1;
}
//通过用户名密码查找一个用户的最高分，不存在此用户返回-1
int32_t mysql::GJMysql::getBestScore(const char *username , const char *password){
    char sql[1024] = {
        '\0'
    };
    sprintf(sql , "select score from UserInfo where username='%s' and password='%s'" , username , password);
    printf("sql: %s\n" , sql);
    if(executesql(sql))
    {
        print_mysql_error(NULL);
        return -1;
    }
    g_res = mysql_store_result(g_conn);
    if(mysql_num_rows(g_res) == 0){
        mysql_free_result(g_res);
        return -1;
    }
    int32_t ans = -1;
    while(g_row = mysql_fetch_row(g_res)){
        printf("select get score: %s\n" , g_row[0]);
        ans = atoi(g_row[0]);
    }
    mysql_free_result(g_res);
    return ans;
}
//通过用户查找这个用户的最高分
int32_t mysql::GJMysql::getBestScore(const char *username){
    char sql[1024] = {
        '\0'
    };
    sprintf(sql , "select score from UserInfo where username='%s'" , username);
    if(executesql(sql))
    {
        print_mysql_error(NULL);
        return -1;
    }
    g_res = mysql_store_result(g_conn);
    if(mysql_num_rows(g_res) == 0){
        mysql_free_result(g_res);
        return -1;
    }
    int32_t ans = -1;
    while(g_row = mysql_fetch_row(g_res)){
        ans = atoi(g_row[0]);
    }
    mysql_free_result(g_res);
    return ans;
}
//通过uid找到玩家的最高分
int32_t mysql::GJMysql::getBestScore(int32_t uid){
    char sql[1024] = {
        '\0'
    };
    sprintf(sql , "select score from UserInfo where uid=%d" , uid);
    if(executesql(sql))
    {
        print_mysql_error(NULL);
        return -1;
    }
    g_res = mysql_store_result(g_conn);
    if(mysql_num_rows(g_res) == 0){
        mysql_free_result(g_res);
        return -1;
    }
    int32_t ans = -1;
    while(g_row = mysql_fetch_row(g_res)){
        ans = atoi(g_row[0]);
    }
    mysql_free_result(g_res);
    return ans;
}
//更新用户的最高分
bool mysql::GJMysql::updateScore(const char *username , int32_t score){
    char sql[1024] = {
        '\0'
    };
    sprintf(sql , "update UserInfo set score=%d where username='%s'" , username);
    if(executesql(sql))
    {
        print_mysql_error(NULL);
        return false;
    }

    return true;
}
//更新用户的最高分通过uid
bool mysql::GJMysql::updateScore(int32_t uid , int32_t score){
    char sql[1024] = {
        '\0'
    };
    sprintf(sql , "update UserInfo set score=%d where uid=%d" , uid);
    if(executesql(sql))
    {
        print_mysql_error(NULL);
        return false;
    }

    return true;
}
//关闭数据库
void mysql::GJMysql::close(){
    mysql_close(g_conn);
}
