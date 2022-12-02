/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Kimheng Peng, Hadi Ramlaoui, Manish Sharma, Rajan Shrestha
* Student IDs: 921847378, 922155413, 922220894, 922223741
* GitHub Name: TotifyKH, HadiRam, mscchapagain, rajshrestha015
* Group Name: File Explorers
* Project: Basic File System
*
* File: test.c
*
* Description: Just a test file to test some of the functions on the fly.
*
**************************************************************/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string.h>


int main(){
char temPath[] = "/a//b//c//////d";
char* pathTokens[64];
pathTokens[0] = NULL;
int tokenIndex = 0;
char* temp[256];
char* delim = "/";

char* token = strtok(temPath, delim);


while(token != NULL){
    if(strcmp(token, "..") == 0){
        if(tokenIndex == 0){
            //DO NTH LIKE '.'
        }else{
            tokenIndex--;
            pathTokens[tokenIndex] = NULL;
        }
        
    }else if(strcmp(token, ".") == 0){
        //DO NTH
    }else{
        pathTokens[tokenIndex] = token;
        tokenIndex++;
    }
    token = strtok(NULL, delim);
}


for(int i = 0; i < tokenIndex; i++){
    printf("%d: %s\n", i, pathTokens[i]);
}

// for(int i = 0; i < strlen(temPath); i++){
//     temp[0] = '\0';

//     while(temPath[i] == '/'){
//         i++;
//     }

//     while(i < strlen(temPath) && temPath[i] != '/'){
     
//     }
// }

}