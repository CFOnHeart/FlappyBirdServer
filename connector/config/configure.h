/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   configure.h
 * Author: ganjungan
 *
 * Created on 2017年1月6日, 下午4:10
 */

#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <stdint.h>
using namespace std;
const int CONFIGBUFFER_SIZE = 100;
namespace Config{

    class Configure{
    private:
        FILE *file;
        map<string , string> mp;
    public:
        Configure();
        Configure(const char *dir);
        void analyse();
        string operator [] (string obj);
        void close();
    };
};

#endif /* CONFIGURE_H */

