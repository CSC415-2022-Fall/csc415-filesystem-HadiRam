#include<stdio.h>
#include<string.h>
#include<stdlib.h>


int main(){

char *pathname = "/home";
if(pathname[0] == '/' && strlen(pathname) <= 1){    
        printf("/");
        return 0;
    }
    
    char* lastElement = "home";
    
    char tempPath[strlen(pathname)+1];
    strcpy(tempPath, pathname);

    
    int len = strlen(lastElement);
    if(strlen(pathname) == len+1){
        tempPath[strlen(pathname) - len] = '\0';
    }else{
        tempPath[strlen(pathname) - len - 1] = '\0';
    }
    
   
    char* path = malloc(strlen(tempPath) + 1);
    memcpy(path, tempPath, strlen(tempPath) + 1);
    
    printf("%s\n", path);

}