#include "file_operation.h"

FileOperation::FileOperation(){

}

FileOperation::~FileOperation(){

}

bool FileOperation::isExistUser(const char *path , const char *username){
    FILE *f = fopen(path , "r");
    char buf[USER_INFO_SIZE] , user[20];
    printf("isExist User: %s\n" , username);
    while(fgets(buf , USER_INFO_SIZE , f)){
        sscanf(buf , "%s" , user);
        if(strcmp(user , username) == 0){
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}

int FileOperation::isValidUser(const char *path , const char *username , const char *pwd) 
{
    FILE *f = fopen(path , "r");
    char buf[USER_INFO_SIZE] , user[20] , password[20] , value[20];
    while(fgets(buf , USER_INFO_SIZE , f)){
        sscanf(buf , "%s%s%s" , user , password , value);
        printf("%s %s %s\n" , user , password , value);
        if(strcmp(user,username)==0 && strcmp(pwd,password)==0){
            fclose(f);
            return atoi(value); 
        }
    }
    fclose(f);
    return -1;
}

void FileOperation::printAllUser(const char *path){
    FILE *fp = fopen(path , "r");
    char buf[USER_INFO_SIZE] = { 0 };
    while(fgets(buf , USER_INFO_SIZE , fp)){
        printf("file get userinfo is: %s" , buf);
    }
    fclose(fp);
}

void FileOperation::addUserInfo(const char *path , const char *username , const char *pwd , int score){
    FILE *f = fopen(path , "a");
    char buf[USER_INFO_SIZE] = { 0 };
    printf("addUserInfo: %s %s %d\n" , username , pwd , score);
    sprintf(buf , "%s %s %d\n" , username , pwd , score);
    fputs(buf , f);
    fclose(f);
}

void FileOperation::updateUserInfo(const char *path , const char *username , int score){
    system("touch tmp");
    FILE *f1 = fopen("tmp" , "w");
    FILE *f2 = fopen(path , "r");

    char buf1[USER_INFO_SIZE] , buf2[USER_INFO_SIZE] , user[20] , pwd[20];
    while(fgets(buf1 , USER_INFO_SIZE , f2)){
        sscanf(buf1 , "%s%s" , user , pwd);
        if(strcmp(user ,username) == 0){
            sprintf(buf2 ,"%s %s %d\n" , user , pwd , score);
            fputs(buf2 , f1);
        }
        else {
            fputs(buf1 , f1);
        }
    }
    fclose(f1);
    fclose(f2);
    system("rm userinfo");
    system("mv tmp userinfo");
}

int FileOperation::getBestScore(const char *path , const char *username){
    FILE *fp = fopen(path , "r");
    char buf[USER_INFO_SIZE] = {0};
    char user[20] , password[20] , value[20];
    int best_score = -1;
    while(fgets(buf , USER_INFO_SIZE , fp)){
       // printf("%s %s\n" , buf , username);
        sscanf(buf , "%s%s%s" , user , password , value);
        printf("we get the info from file : %s %s %s %s\n" , user , password , value , username);
        if(strcmp(username , user) == 0){
            best_score = atoi(value);
            printf("i'm in : %d\n" , best_score );
        }
    }
    fclose(fp);
    return best_score; //return -1 presents can't find the user 
}

void FileOperation::close(FILE *fp){
    fclose(fp);
}
