#include "mfs.h"
#include "b_io.h"
#include "fsLow.h"
#include "dirEntry.h"
#include <string.h>


char *delim = "/";

//function to parse a pathname to check for validity.
//returns error if invalid path (-2)
//returns -1 if path is valid but the
//returns index of n in dir(n-1)
int parsePath(const char *pathname)
{

    char * tempPath = cwdPath;
    dirEntry* tempDirEntries = malloc(51*sizeof(dirEntry));
    char * DEBuffer = malloc(6*512);
    //malloc(51*sizeof(dirEntry));

    //if its absolute, the first character is a slash.
    //Check if path is relative and make it absolute
    if (strcmp(pathname[0], "/") != 0)
    {
        strncat(tempPath, pathname, strlen(pathname));
        tempDirEntries = cwdEntries;
    }else{
        //Grab the root directory entries
        LBAread(DEBuffer, 6, 6);
        memcpy(&tempDirEntries, DEBuffer, 51*sizeof(dirEntry));
    }
    

    //tokenize path with / as delimeter.
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

    //Check if path exists
    int exists = 0;
    int tokenCounter = 0;
    int entryIndex = -2;

    while(pathTokens[tokenCounter] != NULL){
        for(int i = 0; i < 51; i++){
            if(tempDirEntries[i].dirType != -1
             && strcmp(tempDirEntries[i].name, pathTokens[tokenCounter]) == 0){
                exists = 1;
                entryIndex = i;
                i = 51;
            }

        }

        if(exists == 0){
            //Not the last element
            if(tokenCounter != tokenIndex - 1){
                entryIndex = -2;
                break;
            }else{
                printf("The path exists");
                entryIndex = -1;
                break;
            }
        }else if(exists == 1){
            if(tokenCounter != tokenIndex - 1){
                //Load the next directory DE
                LBAread(DEBuffer,6,tempDirEntries[entryIndex].location);
                memcpy(&tempDirEntries, DEBuffer, 51*sizeof(dirEntry));
                exists = 0;
                tokenCounter++;
            }else{
                //Found the file/directory and return the DE index of Dir[n-1]
                break;
            }
        }

    }

    free(tempDirEntries);
    free(DEBuffer);

    return entryIndex;

}


// beginings of getcwd
char *fs_getcwd(char *pathname, size_t size)
{
    return pathname;
}

// begining logic of mkdir.
//int fs_mkdir(const char *pathname, mode_t mode){

//if(path name) is valid, and does not already exist
//initialize new directory
//int numOfDirEntries = 51; //6 blocks
//dirEntry* newDir = malloc(numOfDirEntries*sizeof(dirEntry));

//for(int i = 0; i < numOfDirEntries; i++){
//newDir[i].name = malloc(32*sizeof(char));
//newDir[i].dirType = -1; //free state
//newDir[i].size = 0;
//newDir[i].location = i;
//}
//return 0;

//}