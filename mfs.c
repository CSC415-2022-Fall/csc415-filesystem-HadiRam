#include "mfs.h"
#include "b_io.h"
#include "fsLow.h"
#include "dirEntry.h"
#include <string.h>


//Helper Functions
void loadDirEntries(dirEntry* DEArray, int location){
    char* DEBuffer = malloc(6*512);
    LBAread(DEBuffer, 6, location);
    memcpy(&DEArray, DEBuffer, 51*sizeof(dirEntry));
}

//function that takes a path, and returns the path excluding the last element.
char * getParentDirectory(char *path){
  const char *delim = "/";
  char tempPath[strlen(path)+1];

  strcpy(tempPath, path);
  
  char *token = strtok(tempPath, delim);
  char *pathTokens[64];

  int tokenIndex = 0;
  while (token != NULL)
  {
    pathTokens[tokenIndex] = token;
    token = strtok(NULL, delim);
    tokenIndex++;
  }

  char pathOfParent[64];
  pathOfParent[0] = '\0';
  
  if(path[0] == '/') strcat(pathOfParent, delim);

  for (int i = 0; i < tokenIndex - 1; i++)
  {
    
    strcat(pathOfParent, pathTokens[i]);
    strcat(pathOfParent, delim);
  }

    return pathOfParent;
  

}

//function that gets the last element within a path
//example: pass in "/hadi/desktop/folder", returns "folder".
char * getLastPathElement(char *path){
  const char *delim = "/";
  char tempPath[strlen(path)+1];

  strcpy(tempPath, path);
  
  char *token = strtok(tempPath, delim);
  char *pathTokens[64];

  int tokenIndex = 0;
  while (token != NULL)
  {
    pathTokens[tokenIndex] = token;
    token = strtok(NULL, delim);
    tokenIndex++;
  }

  char * lastPathElement = pathTokens[tokenIndex-1];

  return lastPathElement;
}



//function to parse a pathname to check for validity.
//returns error if invalid path (-2)
//returns -1 if path is valid but the
//returns index of n in dir(n-1)
dirEntry* parsePath(const char *pathname, int* entryIndex)
{
    char *delim = "/";
    char tempPath[strlen(cwdPath)+1];
    strcpy(tempPath,cwdPath);
    dirEntry* tempDirEntries = malloc(51*sizeof(dirEntry));
    char * DEBuffer = malloc(6*512);
    dirEntry* tempDE = malloc(sizeof(tempDE));
    tempDE = NULL;

    //if its absolute, the first character is a slash.
    //Check if path is relative and make it absolute
    if (strcmp(pathname[0], "/") != 0)
    {
        strncat(tempPath, pathname, strlen(pathname));
        tempDirEntries = cwdEntries;
    }else{
        //Grab the root directory entries
        loadDirEntries(tempDirEntries, 6);
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

    while(pathTokens[tokenCounter] != NULL){
        //check if dir is free and the name matches the element within the path.
        //if both are true, initialize the entryIndex, and make exists = 1.
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
                loadDirEntries(tempDirEntries, tempDirEntries[*entryIndex].location);
                exists = 0;
                tokenCounter++;
            }else{
                //Found the file/directory and return the DE index of Dir[n-1]
                tempDE = tempDirEntries[*entryIndex];
                break;
            }
        }

    }

    free(tempDirEntries);
    free(DEBuffer);


    return tempDE;

}


fdDir * fs_opendir(const char *pathname){
    int index;
    dirEntry* tempDE = parsePath(pathname, &index);
    if(index >= 0){
        if(tempDE->dirType != 1){
            printf("Not a directory\n");
            return NULL;
        }
        fdDir* fd = malloc(sizeof(fdDir));
        fd->d_reclen = sizeof(fdDir);
        loadDirEntries(fd->dirPointer, tempDE->location);
        fd->directoryStartLocation = tempDE->location;
        fd->dirEntryPosition = 0;
        return fd;
    }else{
        printf("Invalid path\n");
        return NULL;
    }
}


// beginings of getcwd
char *fs_getcwd(char *pathname, size_t size)
{
    //buffer is pathname, size is size
    //checking if the path fits within the buffer size 
    if(strlen(cwdPath)>size)
    {
        return NULL;
    }
    strcpy(pathname,cwdPath);
    return pathname;
}

//setting current working directory
int fs_setcwd(char *pathname)
{
    //check if the path exists
    int index;
    dirEntry* cwdDE = parsePath(pathname, &index);

    if(index >= 0){ 

    //set both global variables

    //setting cwdEntries
    //lba read into buffer, dirEntry.location.
    loadDirEntries(cwdEntries, cwdDE->location);

    //setting cwdPath
    strcpy(cwdPath,pathname);
    
    //success
    return 0;
    }

    //if fails
    return -1;
}

int fs_mkdir(const char *pathname, mode_t mode){

//use parsepath to check
//dirEntry dir = parsePath(pathname, 0);
//char* test = dir.dirType;

//Get the path excluding the last directory/file.
char * parentPath = getParentDirectory(pathname);

//Get the last element in the path
char * lastElementOfPath = getLastPathElement(pathname);

//Set the parent path to the current working directory.
fs_setcwd(parentPath);

//look for a free directory entry,by looping through cwdEntries[i].dirType = -1
//once found, set the name, to new file name (last element of passed path)
//set dir type type 1, set size to sizeOf(sizeof(dirEntry) * numOfDirEntries)
//set location to 6 free blocks, using bitmap functions which returns index.
//Also lbawrite to that index which is the location of the 6 free blocks.
//cwdEntries[0] is . (itself), which will be set to the parent of the new directory ..

int numOfDirEntries = 51; //6 blocks
dirEntry* newDir = malloc(numOfDirEntries*sizeof(dirEntry));
//get directory of n-1, u wanna make it in n 
for(int i = 0; i < numOfDirEntries; i++){
newDir[i].name = malloc(32*sizeof(char));
newDir[i].dirType = -1; //free state
newDir[i].size = 0;
newDir[i].location = i;
}

}

