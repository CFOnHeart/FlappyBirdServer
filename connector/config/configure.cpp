/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "configure.h"

Config::Configure::Configure(){
    file = NULL;
    mp.clear();
}
Config::Configure::Configure(const char *path){
    file = fopen(path , "r");
    mp.clear();
}
void Config::Configure::analyse(){
    char buffer[CONFIGBUFFER_SIZE];
    char key[CONFIGBUFFER_SIZE] , value[CONFIGBUFFER_SIZE];
    while(fgets(buffer , CONFIGBUFFER_SIZE , file)){
        if(buffer[0] == '#') continue; //#表示注释
      //  printf("Debug configure.cpp: %s\n" , buffer);
        sscanf(buffer , "%s : %s" , key , value);
     //   printf("Debug configure.cpp: %s %s\n" , key,value);
        mp[key] = value;
    }
}
string Config::Configure::operator [] (string obj){
    return mp[obj];
}
void Config::Configure::close(){
    fclose(file);
}