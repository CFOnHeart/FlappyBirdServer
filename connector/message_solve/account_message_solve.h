/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   login_message_solve.h
 * Author: ganjungan
 *
 * Created on 2017年1月6日, 下午2:39
 */

#ifndef LOGIN_MESSAGE_SOLVE_H
#define LOGIN_MESSAGE_SOLVE_H

#include "../datalayer/sql_operation.h"
namespace message_solve{
    //暂时这个账户消息类只处理登录和注册的消息，消息格式都是username&password
    //暂时认定的是网关处理数据库逻辑，所以网关服务器启动的时候就连接mysql，
    //网关服务器关闭的时候断开mysql，这样只连接了一次mysql
    class AccountMessage{
    public:
        static mysql::GJMysql gjmysql;
        AccountMessage();
        ~AccountMessage();
        
        static char username[20];
        static char password[20];
        
        static void init_mysql();
        
        static void close_mysql();
        
        static void analyseMessage(const char *message);
        
        static bool isValidUser(const char *username , const char *password);
        
        static bool isValidUser(const char *message);
        
        static bool isExistUser(const char *username);
        
        static int32_t getBestScore(const char *username);
        
        static int32_t getBestScore(const char *username , const char *password);
        
        static int32_t getBestScore(int32_t uid);
         
        static bool addUserInfo(int32_t uid , const char *username , const char *password , int32_t score);
        
        static bool updateScore(const char *username , int32_t score);

        static bool updateScore(int32_t uid , int32_t score);
    };
};

#endif /* LOGIN_MESSAGE_SOLVE_H */

