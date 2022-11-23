#include "mfs.h"
#include "b_io.h"
#include "fsLow.h"
#include "dirEntry.h"
#include "vcb.h"
#include "bitMap.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//Helper Functions
void loadDirEntries(dirEntry* DEArray, int location){
    char* DEBuffer = malloc(DIRECTORY_BLOCKSIZE*512);
    LBAread(DEBuffer, DIRECTORY_BLOCKSIZE, location);
    memcpy(DEArray, DEBuffer, MAX_DIRENT_SIZE*sizeof(dirEntry));
}

void initGlobalVar(){
    cwdPath = malloc(256);
    strcpy(cwdPath, "/");

    cwdEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
    char* DEBuffer = malloc(DIRECTORY_BLOCKSIZE*512);
    LBAread(DEBuffer, DIRECTORY_BLOCKSIZE, vcb.RootDir);
    memcpy(cwdEntries, DEBuffer, MAX_DIRENT_SIZE*sizeof(dirEntry));
    
}

//function that takes a path, and returns the path excluding the last element.
char * getParentDirectory(const char *pathname){
    char* path;
    strcpy(path, pathname);
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

    char * result;
    strcpy(result, pathOfParent);
    return result;
  
}

//function that gets the last element within a path
//example: pass in "/hadi/desktop/folder", returns "folder".
char * getLastPathElement(const char *pathname){
    char* path;
    strcpy(path, pathname);
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
pathInfo* parsePath(const char *pathname)
{
    int entryIndex = 0;
    char *delim = "/";
    char tempPath[strlen(pathname)+1];
    strcpy(tempPath, pathname);
    
    dirEntry* tempDirEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
    char * DEBuffer = malloc(DIRECTORY_BLOCKSIZE*512);
    

    dirEntry* tempDE= malloc(sizeof(tempDE));
    tempDE = NULL;

    //if its absolute, the first character is a slash.
    //Check if path is relative and make it absolute
    if (pathname[0] != '/')
    {
        strcpy(tempPath, cwdPath);
        strncat(tempPath, pathname, strlen(pathname));
        tempDirEntries = cwdEntries;
    }else{
        //Grab the root directory entries   
        loadDirEntries(tempDirEntries, vcb.RootDir);   
    }
    
    char tempPathArr[strlen(tempPath) + 1];
    strcpy(tempPathArr, tempPath);
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
    for(int i = 0; i < tokenIndex; i++){
        printf("Path:%s\n", pathTokens[i]);
    }
   printf("ParsePath completed");
   
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
                //Path does exist but element does not exist
                entryIndex = -1;
                break;
            }
        }else if(exists == 1){
            if(tokenCounter != tokenIndex - 1){
                //Load the next directory DE
                loadDirEntries(tempDirEntries, tempDirEntries[entryIndex].location);
                exists = 0;
                tokenCounter++;
            }else{
                //Found the file/directory and return the DE index of Dir[n-1]
                tempDE = &tempDirEntries[entryIndex];
                break;
            }
        }

    }
     
    free(tempDirEntries);
    free(DEBuffer);

    pathInfo* result = malloc(sizeof(pathInfo));

    result->DEPointer = tempDE;

    result->value = entryIndex;
    strcpy(result->path,pathname);

   

    return result;

}
// dirEntry* parsePath(const char *pathname, int* entryIndex)
// {
    
//     char *delim = "/";
//     char tempPath[strlen(pathname)+1];
//     strcpy(tempPath, pathname);
    
//     dirEntry* tempDirEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
//     char * DEBuffer = malloc(DIRECTORY_BLOCKSIZE*512);
    

//     dirEntry* tempDE= malloc(sizeof(tempDE));
//     tempDE = NULL;
    
    

//     //if its absolute, the first character is a slash.
//     //Check if path is relative and make it absolute
//     if (pathname[0] != '/')
//     {
//         strcpy(tempPath, cwdPath);
//         strncat(tempPath, pathname, strlen(pathname));
//         tempDirEntries = cwdEntries;
//     }else{
//         //Grab the root directory entries   
//         loadDirEntries(tempDirEntries, vcb.RootDir);   
//     }
    
//     char tempPathArr[strlen(tempPath) + 1];
//     strcpy(tempPathArr, tempPath);
//     //tokenize path with / as delimeter.
//     char *token = strtok(tempPath, delim);
//     char *pathTokens[64];

//     int tokenIndex = 0;
//     while (token != NULL)
//     {
//         pathTokens[tokenIndex] = token;
//         token = strtok(NULL, delim);
//         tokenIndex++;
//     }

//     pathTokens[tokenIndex] = NULL;

//     //Check if path exists
//     int exists = 0;
//     int tokenCounter = 0;
//     for(int i = 0; i < tokenIndex; i++){
//         printf("Path:%s\n", pathTokens[i]);
//     }
   
//     while(pathTokens[tokenCounter] != NULL){
//         //check if dir is free and the name matches the element within the path.
//         //if both are true, initialize the entryIndex, and make exists = 1.
//         for(int i = 0; i < 51; i++){
            
//             if(tempDirEntries[i].dirType != -1
//              && strcmp(tempDirEntries[i].name, pathTokens[tokenCounter]) == 0){
//                 exists = 1;
//                 *entryIndex = i;
//                 i = 51;
                 
//             }

//         }

//         if(exists == 0){
//             //Not the last element
//             if(tokenCounter != tokenIndex - 1){
//                 *entryIndex = -2;
//                 break;
//             }else{
//                 //Path does exist but element does not exist
//                 *entryIndex = -1;
//                 break;
//             }
//         }else if(exists == 1){
//             if(tokenCounter != tokenIndex - 1){
//                 //Load the next directory DE
//                 loadDirEntries(tempDirEntries, tempDirEntries[*entryIndex].location);
//                 exists = 0;
//                 tokenCounter++;
//             }else{
//                 //Found the file/directory and return the DE index of Dir[n-1]
//                 tempDE = &tempDirEntries[*entryIndex];
//                 break;
//             }
//         }

//     }

//     free(tempDirEntries);
//     free(DEBuffer);

    

//     return tempDE;

// }


fdDir * fs_opendir(const char *pathname){
    pathInfo* pi = parsePath(pathname);
    if(pi->value >= 0){
        if(pi->DEPointer->dirType != 1){
            printf("Not a directory\n");
            return NULL;
        }
        fdDir* fd = malloc(sizeof(fdDir));

        fd->dirPointer = malloc(51*sizeof(dirEntry));
        loadDirEntries(fd->dirPointer, pi->DEPointer->location);

        fd->d_reclen = sizeof(fdDir);
        fd->directoryStartLocation = pi->DEPointer->location;
        fd->dirEntryPosition = 0;
        fd->dirSize = (pi->DEPointer->size)/((int) sizeof(dirEntry));
        return fd;
    }else{
        printf("Invalid path\n");
        return NULL;
    }
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    struct fs_diriteminfo* ii;
    ii = NULL;

    for(int i = dirp->dirEntryPosition; i < dirp->dirSize; i++){
        if(dirp->dirPointer[i].dirType != -1){
            strcpy(ii->d_name, dirp->dirPointer[i].name);
            ii->d_reclen = (int) sizeof(struct fs_diriteminfo);
            if(dirp->dirPointer[i].dirType == 1){
                ii->fileType = '1';
            }else{
                ii->fileType = '0';
            }
            dirp->dirEntryPosition += 1;
            //EXIT LOOP CONDITION
            i = dirp->dirSize;
        }
    }

    return ii;
}

int fs_closedir(fdDir *dirp){
    free(dirp->dirPointer);
    free(dirp);
    dirp = NULL;
}

// beginings of getcwd
char *fs_getcwd(char *pathname, size_t size)
{
    //buffer is pathname, size is size
    
    strncpy(pathname,cwdPath,size);
    return pathname;
}

//setting current working directory
int fs_setcwd(char *pathname)
{
    // cd John
    //cwd a/b/John
    //check if the path exists

    if(strcmp(pathname, "./") == 0){

        return 0;
    }
    
    pathInfo* pi = parsePath(pathname);
    
    if(pi->value >= 0){ 

    //set both global variables

    //setting cwdEntries
    //lba read into buffer, dirEntry.location.
    loadDirEntries(cwdEntries, pi->DEPointer->location);

    //setting cwdPath
    //TODO
    char * temp = malloc(32);
    temp = getParentDirectory(cwdPath);
    strcpy(cwdPath,pi->path);
    
    //success
    return 0;
    }

    //if fails
    return -1;
}


//returns 0 if directory was succesfully created, at specified path.
//returns -1 if there was an error and directory was not created.
int fs_mkdir(const char *pathname, mode_t mode)
{

    // Get the path excluding the last directory/file.
    char *parentPath = getParentDirectory(pathname);

    // Get the last element in the path
    char *lastElementOfPath = getLastPathElement(pathname);

    // Set the parent path to the current working directory.
    if (fs_setcwd(parentPath) == -1)
    {
        printf("Error, failed to set parent path as current working directory.");
        return -1;
    }

    // cwdEntries is set using the parent path, now these entries will be
    // looped through to find an entry ina  free state.

    int numOfDirEntries = 51;
    int indexOfNewDirEntry;

    for (int i = 0; i < numOfDirEntries; i++)
    {
        // if the dir is free, begin creating the new directory.
        if (cwdEntries[i].dirType == -1)
        {
            indexOfNewDirEntry = i;
            strcpy(cwdEntries[i].name, lastElementOfPath);
            cwdEntries[i].dirType = 1;
            cwdEntries[i].size = (int)(sizeof(dirEntry) * 2);
            cwdEntries[i].location =
                getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, 6);
            time(&cwdEntries[i].created);
            time(&cwdEntries[i].lastModified);
            cwdEntries[0].size += (int)sizeof(dirEntry);
            i = 52;
        }
    }

    // Parse path on passed path, to get the directry entry of the new directroy.
    dirEntry *dirEntries = malloc(51 * sizeof(dirEntry));
    loadDirEntries(dirEntries, cwdEntries[indexOfNewDirEntry].location);

    for (int i = 0; i < numOfDirEntries; i++)
    {
        dirEntries[i].name[0] = '\0';
        dirEntries[i].dirType = -1; // free state
        dirEntries[i].size = 0;
        dirEntries[i].location = -1;
    }

    // Set up the "." Directory Entry
    strcpy(dirEntries[0].name, ".");
    dirEntries[0].size = (int)(sizeof(dirEntry) * 2);
    // 1 for Directory type directory Entry
    dirEntries[0].dirType = 1;
    dirEntries[0].location = cwdEntries[indexOfNewDirEntry].location;
    // Setting the time created and last modified to the current time
    time(&dirEntries[0].created);
    time(&dirEntries[0].lastModified);

    // Set up the ".." Directory Entry, repeat the step
    strcpy(dirEntries[1].name, "..");
    dirEntries[1].size = cwdEntries[0].size;
    // 1 for Directory type directory Entry
    dirEntries[1].dirType = cwdEntries[0].dirType;
    dirEntries[1].location = cwdEntries[0].location;
    // Setting the time created and last modified to the current time
    dirEntries[1].created = cwdEntries[0].created;
    dirEntries[1].lastModified = cwdEntries[0].lastModified;

    // Writing the cwd and new dir entry to disk
    LBAwrite(dirEntries, 6, dirEntries[0].location);
    LBAwrite(cwdEntries, 6, cwdEntries[indexOfNewDirEntry].location);

    // return 0 on successful creation of new directory.
    return 0;
}

//---------------isFile----------------------
//return 1 if file, 0 otherwise
int fs_isFile(char * filename)
{
    //if filename is a path, we have to parse it
   

    pathInfo* pi = parsePath(filename);

    if(pi->value >= 0)
    {
        if(pi->DEPointer->dirType == 0)
        {
            printf("It is a valid file\n");
            return 1;
        }
        else {
            printf("Not a file\n");
            return 0;
        }
    } else {
        printf("Invalid path\n");
        return -1;
    }

    
}


//------------- fs_isDir --------------
//return 1 if directory, 0 otherwise
int fs_isDir(char * pathname)
{
    int index = fs_isFile(pathname);

    if (index == 1)
    {
        printf("Not a directory. Its a file\n");
        return 0;
    }
    else if(index == 0) {
        printf("Its a directory.\n");
        return 1;
    }
    else {
        printf("Invalid path\n");
        return -1;
    }


}


//----------------fs_stat()----------------
int fs_stat(const char *path, struct fs_stat *buf)
{
    //first parse the path
     
    pathInfo* pi = parsePath(path);
    
    //path is valid
    if (pi->value >= 0){
        loadDirEntries(cwdEntries,pi->DEPointer->location);
        buf->st_accesstime = cwdEntries->lastModified;
        buf->st_size = cwdEntries->size;
        buf->st_createtime = cwdEntries->created;
        buf->st_modtime = cwdEntries->lastModified;

        return 1; //success
    }

    return -1;   //on failure
}

//----GetFileInfo()-------------

dirEntry * GetFileInfo (char * fname)
{
    pathInfo* pi = parsePath(fname);

    if(pi->value >=0){
        return pi->DEPointer;
    }

    return NULL; //error
}