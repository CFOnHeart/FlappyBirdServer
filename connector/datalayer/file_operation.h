#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
const int USER_INFO_SIZE = 64;
const char *PATH = "userinfo";
class FileOperation{
public:  
    FileOperation();
    ~FileOperation();
  
    bool isExistUser(const char *path , const char *username); //判断是否存在相同的用户名，保证用户名的唯一
    int isValidUser(const char * path , const char *username , const char *pwd); //return the user's score , -1 presents not exist
    void printAllUser(const char *path);
    void addUserInfo(const char *path , const char *username , const char *pwd , int score);
    void updateUserInfo(const char *path , const char *username , int score);
    int getBestScore(const char *path , const char *username);
    static void close(FILE *fp);
};
