#include<stdio.h>
#include<string.h>
#include<stdlib.h>


int main(){

char string[] = "/Home/Desktop";
char *sub = "Desktop";

    int len = strlen(sub);
    int len1 = strlen(string);
    string[len1 - len] = '\0';
    printf("%s\n", string);
}