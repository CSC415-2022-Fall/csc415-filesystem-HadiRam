#include "mfs.h"
#include "b_io.h"
#include "fsLow.h"
#include <string.h>

char *delim = "/";

//function to parse a pathname to check for validity.
//returns error if invalid path (-2)
//returns -1 if path is valid but the
//returns index of n in dir(n-1)
int parsePath(const char *pathname)
{

    char * tempPath = cwdPath;
    dirEntry* tempDirEntries = cwdEntries;
    //malloc(51*sizeof(dirEntry));

    //check if path is relative or absolute
    //if its absolute, the first character is a slash.

    //if equal to zero, path is absolute, else path is relative
    if (strcmp(pathname[0], "/") == 0)
    {
    }
    //if its not absolute path, append working directory.
    else
    {
        strncat(tempPath, pathname, strlen(pathname));
       //tempPath = cwdPath + tempPath;
    }

    //tokenize path with / as delimeter.
    char *token = strtok(tempPath, delim);
    char *pathTokens[32];

    int counter = 0;
    while (token != NULL)
    {
        pathTokens[counter] = token;
        token = strtok(NULL, delim);
        counter++;
    }

    //Check if path exists
    int exists = 0;
    int tokenCounter = 0;
    int entryIndex = 0;
    while(pathTokens[tokenCounter] != NULL){
        for(int i = 0; i < 51; i++){
            if(tempDirEntries[i].dirType != -1
             && strcmp(tempDirEntries[i].name, pathTokens[tokenCounter]) == 0){
                exists = 1;
                entryIndex = i;
                i = 51;
            }

        }
        if(exists == 0 && counter != 0){
            printf("Error invalid path");
        }else{
            counter--;
            LBAread(tempDirEntries,6,tempDirEntries[entryIndex].location);
            exists = 0;
            tokenCounter++;
        }
        


    }





}


// beginings of getcwd
/*char *fs_getcwd(char *pathname, size_t size)
{
    //if parsepath(pathname) retruns 1. 

    //if its root return /
    char *currentWorkingDirectory = pathname;

    

    



    return  currentWorkingDirectory;
}*/

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