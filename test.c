#include<stdio.h>
#include<string.h>
#include<stdlib.h>


int main(){

char *pathname = "./home";
char* tempPath[256];
strcpy(tempPath, pathname);

printf("%s, %d\n", tempPath, strlen(tempPath));

}