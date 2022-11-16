#include<stdio.h>
#include<string.h>
#include<stdlib.h>


int main(){
    printf("Hello world");
char tempPath[] = "/Home/Desktop";
char * delim = "/";
char *token = strtok(tempPath, delim);
    char *pathTokens[64];

    int tokenIndex = 0;
    while (token != NULL)
    {
        pathTokens[tokenIndex] = token;
        token = strtok(NULL, delim);
        tokenIndex++;
    }

    pathTokens[tokenIndex] = NULL;
}