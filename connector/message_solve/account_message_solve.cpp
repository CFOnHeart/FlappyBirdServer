/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "account_message_solve.h"

mysql::GJMysql message_solve::AccountMessage::gjmysql = mysql::GJMysql();
char message_solve::AccountMessage::username[20] = {0};                                                                                                                                                     
char message_solve::AccountMessage::password[20] = {0}; 

message_solve::AccountMessage::AccountMessage(){
    memset(username , 0 , sizeof(username));
    memset(password , 0 , sizeof(password));
}

message_solve::AccountMessage::~AccountMessage(){
    
}

void message_solve::AccountMessage::init_mysql(){
    gjmysql.init_config();
    gjmysql.init_mysql();
  //  printf("Debug finish mysql init: %s %s %s %s\n" , gjmysql.MYSQLIP , gjmysql.USER , gjmysql.PASSWORD , gjmysql.DATABASE);
}

void message_solve::AccountMessage::close_mysql(){
    gjmysql.close();
}

void message_solve::AccountMessage::analyseMessage(const char* message){
    memset(username , 0 , sizeof(username));
    memset(password , 0 , sizeof(password));
   // int len = strlen(message);
    const char *pos = strchr(message , '&');
    strncpy(username , message , pos-message);
    strcpy(password , pos+1);
}

bool message_solve::AccountMessage::isValidUser(const char *username , const char *password){
    return gjmysql.isValidUser(username , password);
}

bool message_solve::AccountMessage::isValidUser(const char *message){
    analyseMessage(message);
    return isValidUser(username , password);
}

bool message_solve::AccountMessage::isExistUser(const char *username){
    return gjmysql.isExistUser(username);
}

int32_t message_solve::AccountMessage::getBestScore(const char* username){
    return gjmysql.getBestScore(username);
}

int32_t message_solve::AccountMessage::getBestScore(const char *username , const char *password){
    return gjmysql.getBestScore(username , password);
}

int32_t message_solve::AccountMessage::getBestScore(int32_t uid){
    return gjmysql.getBestScore(uid);
}

bool message_solve::AccountMessage::addUserInfo(int32_t uid , const char *username , const char *password , int32_t score){
    return gjmysql.addUserInfo(uid , username , password , score);
}

bool message_solve::AccountMessage::updateScore(const char *username , int32_t score){
    return gjmysql.updateScore(username , score);
}

bool message_solve::AccountMessage::updateScore(int32_t uid , int32_t score){
    return gjmysql.updateScore(uid , score);
}

