#include<stdio.h>
#include<string.h>
#include<stdlib.h>


int main(){
char tempPath[] = "../";
char *str;
str = strrchr(tempPath, '/');
if (str != NULL) {
    str++; /* step over the slash */
    printf("%s\n", str);
}else{
    printf("Hello\n");
}

}