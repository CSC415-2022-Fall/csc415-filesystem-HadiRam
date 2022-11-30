#include "mfs.h"
#include "b_io.h"
#include "fsLow.h"
#include "dirEntry.h"
#include "vcb.h"
#include "bitMap.h"
#include "extent.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Initializing extern Global variables from mfs.h

char* cwdPath;
dirEntry* cwdEntries;


//Helper Functions


void loadDirEntries(dirEntry* DEArray, int location){
    LBAread(DEArray, DIRECTORY_BLOCKSIZE, location);
}

void initGlobalVar(){
    cwdPath = malloc(256);
    strcpy(cwdPath, "/");

    cwdEntries = malloc(MAX_DIRENT_SIZE*DE_STRUCT_SIZE);
    char* DEBuffer = malloc(DIRECTORY_BLOCKSIZE*512);
    LBAread(DEBuffer, DIRECTORY_BLOCKSIZE, vcb.RootDir);
    memcpy(cwdEntries, DEBuffer, MAX_DIRENT_SIZE*DE_STRUCT_SIZE);
    
}

//function that gets the last element within a path
//example: pass in "/hadi/desktop/folder", returns "folder".
//return NULL if path invalid
char * getLastPathElement(const char *pathname){

    char *str;
    //strrchr returns the pointer to the last slash("/")
    str = strrchr(pathname, '/');
    //Getting pass the last slash
    if (str != NULL) {
        str++; 
    }

    return str;
}

//function that takes a path, and returns the path excluding the last element.
char * getParentDirectory(const char *pathname){
    if(pathname[0] == '/' && strlen(pathname) <= 1){
        char* path = malloc(sizeof(char)*2);
        strcpy(path, pathname);

        return path;
    }

    char* lastElement = getLastPathElement(pathname);
    
    char tempPath[strlen(pathname)+1];
    strcpy(tempPath, pathname);

    
    int len = strlen(lastElement);
    //In the case the parent is the root "/home" => "/"
    if(strlen(pathname) == len+1){
        tempPath[strlen(pathname) - len] = '\0';
    }else{
        //regular case where we would want to remove last slash
        //Example: "/home/foo" => "/home"
        tempPath[strlen(pathname) - len - 1] = '\0';
    }
   
    char* path = malloc(strlen(tempPath) + 1);
    memcpy(path, tempPath, strlen(tempPath) + 1);
    
    return path;

}





//function to parse a pathname to check for validity.
//returns error if invalid path (-2)
//returns -1 if path is valid but the
//returns index of n in dir(n-1)
pathInfo* parsePath(const char *pathname)
{
    pathInfo* result = malloc(sizeof(pathInfo));
    result->DEPointer = malloc(sizeof(dirEntry));

    int entryIndex = 0;
    char *delim = "/";
    char tempPath[256];
    tempPath[0] = '\0';
    strcpy(tempPath, pathname);
    
    dirEntry* tempDirEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
    char * DEBuffer = malloc(DIRECTORY_BLOCKSIZE*512);

    
    
    //if its absolute, the first character is a slash.
    //Check if path is relative and make it absolute
    if (pathname[0] != '/')
    {
        strcpy(tempPath, cwdPath);
        strcat(tempPath, "/");
        strncat(tempPath, pathname, strlen(pathname));    
    }
    
    loadDirEntries(tempDirEntries, vcb.RootDir);  
    
    //tokenize path with / as delimeter.
    char *token = strtok(tempPath, delim);
    char *pathTokens[64];
    pathTokens[0] = NULL;

    int tokenIndex = 0;
    
    while(token != NULL){
        if(strcmp(token, "..") == 0){
            if(tokenIndex == 0){
                //DO NTH LIKE '.'
            }else{
                //Pop the top of the stack
                tokenIndex--;
                pathTokens[tokenIndex] = NULL;
            }
                
        }else if(strcmp(token, ".") == 0){
            //DO NTH
        }else{
            //Add to the stack
            pathTokens[tokenIndex] = token;
            tokenIndex++;
        }
        token = strtok(NULL, delim);
        }
    //Check if the path is just "/"
    if(tokenIndex == 0){
        
        char* temp = ".";
        pathTokens[0] = temp;
        tokenIndex++;
        
    }
    pathTokens[tokenIndex] = NULL;
    
    char* absolutePath = malloc(64);
    absolutePath[0] = '\0';
    for(int i = 0; i < tokenIndex; i++){
        if(strcmp(pathTokens[0], ".") == 0){
            strcat(absolutePath, delim);
        }else{
            strcat(absolutePath, delim);
            strcat(absolutePath, pathTokens[i]);
        }
    }

    strcpy(result->path, absolutePath);
    
    int exists = 0;
    int tokenCounter = 0;
    // for(int i = 0; i < tokenIndex; i++){
    //     printf("Path:%s\n", pathTokens[i]);
    // }
    
   
    while(pathTokens[tokenCounter] != NULL){
        //check if dir is free and the name matches the element within the path.
        //if both are true, initialize the entryIndex, and make exists = 1.
        for(int i = 0; i < MAX_DIRENT_SIZE; i++){
            
            if(tempDirEntries[i].dirType != -1
             && strcmp(tempDirEntries[i].name, pathTokens[tokenCounter]) == 0){
                exists = 1;
                entryIndex = i;
                
                i = MAX_DIRENT_SIZE;
            }

        }

        if(exists == 0){
            //Not the last element

            if(tokenCounter != tokenIndex - 1){
                entryIndex = -2;
                result->DEPointer = NULL;
                break;
            }else{
                //Path does exist but element does not exist
                entryIndex = -1;
                result->DEPointer = NULL;
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
               
                strcpy(result->DEPointer->name, tempDirEntries[entryIndex].name);
                result->DEPointer->created = tempDirEntries[entryIndex].created;
                result->DEPointer->size = tempDirEntries[entryIndex].size;
                result->DEPointer->dirType = tempDirEntries[entryIndex].dirType;
                result->DEPointer->extentLocation = tempDirEntries[entryIndex].extentLocation;
                result->DEPointer->lastModified = tempDirEntries[entryIndex].lastModified;
                result->DEPointer->location = tempDirEntries[entryIndex].location;

                break;
            }
        }

    }
    
    free(tempDirEntries);
    free(DEBuffer);

    result->value = entryIndex;
    
    //printf("Mypath: %s\n", result->path);
    return result;

}



fdDir * fs_opendir(const char *pathname){
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(pathname);

    if(pi->value >= 0){
        if(pi->DEPointer->dirType != 1){
            printf("Not a directory\n");
            return NULL;
        }
        fdDir* fd = malloc(sizeof(fdDir));

        fd->dirPointer = malloc(MAX_DIRENT_SIZE*DE_STRUCT_SIZE);
        printf("Location:%d, size:%d\n", pi->DEPointer->location, pi->DEPointer->size);
        loadDirEntries(fd->dirPointer, pi->DEPointer->location);
        fd->d_reclen = sizeof(fdDir);
        fd->directoryStartLocation = pi->DEPointer->location;
        fd->dirEntryPosition = 0;
        fd->dirSize = (fd->dirPointer[0].size)/DE_STRUCT_SIZE;
        fd->fileIndex = 0;
        return fd;
    }else{
        printf("1.Invalid path\n");
        return NULL;
    }
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    struct fs_diriteminfo* ii = malloc(sizeof(struct fs_diriteminfo));
    int exist = 0;

    if(dirp->fileIndex == dirp->dirSize){
        return NULL;
    }
    for(int i = dirp->dirEntryPosition; i < MAX_DIRENT_SIZE; i++){
        if(dirp->dirPointer[i].dirType == 0 || dirp->dirPointer[i].dirType == 1){
            strcpy(ii->d_name, dirp->dirPointer[i].name);
            //DEBUG
            //printf("%s and %d\n",dirp->dirPointer[i].name,dirp->dirPointer[i].location);
            ii->d_reclen = (int) sizeof(struct fs_diriteminfo);
            if(dirp->dirPointer[i].dirType == 1){
                ii->fileType = '1';
            }else{
                ii->fileType = '0';
            }
            
            dirp->fileIndex++;
            
            //Exit loop
            i = MAX_DIRENT_SIZE;
            exist = 1;
        }
        dirp->dirEntryPosition++;
    }
    if(exist == 0){
        ii = NULL;
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
    printf("set new dir\n");
    //Write current working directory back to disk
    LBAwrite(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);

    if(strcmp(pathname, "./") == 0 ){

        return 0;
    }
    
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(pathname);
    
    if(pi->value >= 0){ 
        if(pi->DEPointer->dirType != 1){
            return -1;
        }
        //setting cwdEntries
        //lba read into buffer, dirEntry.location.
        
        loadDirEntries(cwdEntries, pi->DEPointer->location);
        
        //setting cwdPath   
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
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(pathname);
    // Get the path excluding the last directory/file.
    if(pi->value != -1){
        return -1;
    }
    char *parentPath = getParentDirectory(pi->path);
    
    // Get the last element in the path
    char *lastElementOfPath = getLastPathElement(pi->path);
    // Set the parent path to the current working directory.
    
    if (fs_setcwd(parentPath) == -1)
    {
        printf("Error, failed to set parent path as current working directory.");
        return -1;
    }

    // cwdEntries is set using the parent path, now these entries will be
    // looped through to find an entry ina  free state.

    int numOfDirEntries = MAX_DIRENT_SIZE;
    int indexOfNewDirEntry;

    for (int i = 0; i < numOfDirEntries; i++)
    {
        // if the dir is free, begin creating the new directory.
        if (cwdEntries[i].dirType == -1)
        {
            indexOfNewDirEntry = i;
            strcpy(cwdEntries[i].name, lastElementOfPath);
            cwdEntries[i].dirType = 1;
            cwdEntries[i].size = DE_STRUCT_SIZE*2;
            cwdEntries[i].location =
                getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, DIRECTORY_BLOCKSIZE);
            updateBitMap(vcb.freeSpaceBitMap);
            time(&cwdEntries[i].created);
            time(&cwdEntries[i].lastModified);
            cwdEntries[0].size += DE_STRUCT_SIZE;
            //Update .. if its the root directory
            if(cwdEntries[0].location == cwdEntries[1].location){
                cwdEntries[1].size += DE_STRUCT_SIZE;
            }else{
                char* parentDir = getLastPathElement(cwdPath);
                dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
                LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
                for(int i = 0; i < MAX_DIRENT_SIZE; i++){
                    if(strcmp(parentDir, tempDEntries[i].name) == 0){
                        tempDEntries[i].size += DE_STRUCT_SIZE;
                        //Exit loop
                        i = MAX_DIRENT_SIZE;
                    }
                }
                LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
            }
            i = 52;
        }
    }
    
    
    // Parse path on passed path, to get the directry entry of the new directroy.
    dirEntry *dirEntries = malloc(MAX_DIRENT_SIZE * DE_STRUCT_SIZE);
    loadDirEntries(dirEntries, cwdEntries[indexOfNewDirEntry].location);

    for (int i = 0; i < numOfDirEntries; i++)
    {
        dirEntries[i].name[0] = '\0';
        dirEntries[i].dirType = -1; // free state
        dirEntries[i].size = 0;
        dirEntries[i].location = -1;
        dirEntries[i].extentLocation = -1;
    }

    // Set up the "." Directory Entry
    strcpy(dirEntries[0].name, ".");
    dirEntries[0].size = DE_STRUCT_SIZE*2;;
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
    // Setting the time createdand last modified to the current time
    dirEntries[1].created = cwdEntries[0].created;
    dirEntries[1].lastModified = cwdEntries[0].lastModified;

    // Writing the cwd and new dir entry to disk 
    LBAwrite(dirEntries, DIRECTORY_BLOCKSIZE, dirEntries[0].location);
    LBAwrite(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);

    // return 0 on successful creation of new directory.
    return 0;
}

//---------------isFile----------------------
//return 1 if file, 0 otherwise
int fs_isFile(char * filename)
{
    //if filename is a path, we have to parse it
   
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(filename);

    if(pi->value >= 0)
    {
        if(pi->DEPointer->dirType == 0)
        {           
            return 1;
        }
        else {
            return 0;
        }
    } else {
        return 0;
    }

    
}


//------------- fs_isDir --------------
//return 1 if directory, 0 otherwise
int fs_isDir(char * pathname)
{   
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(pathname);

    if(pi->value >= 0)
    {
        if(pi->DEPointer->dirType == 1)
        {           
            return 1;
        }
        else {
            return 0;
        }
    } else {
        return 0;
    }


}


//----------------fs_stat()----------------
int fs_stat(const char *path, struct fs_stat *buf)
{
    //first parse the path
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(path);
    
    //path is valid
    if (pi->value >= 0){
        buf->st_accesstime = pi->DEPointer->lastModified;
        buf->st_size = pi->DEPointer->size;
        buf->st_createtime = pi->DEPointer->created;
        buf->st_modtime = pi->DEPointer->lastModified;

        return 1; //success
    }

    return -1;   //on failure
}


int fs_rmdir(const char *pathname){
    if(pathname[0] == '.'){
        printf("Directory cannot be removed\n");
        return -1;
    }
    int isEmpty = 0;
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(pathname);
    if(pi->DEPointer->dirType != 1){
        printf("Is not a directory\n");
        return -1;
    }
    //printf("TEST: %s, %d, %d\n",pi->path, pi->DEPointer->size, DE_STRUCT_SIZE*2);
    if(pi->DEPointer->size != DE_STRUCT_SIZE*2){
        isEmpty = 1;
    }

    if(isEmpty == 1){
        printf("Directory is not empty!\n");
        return -1;
    }

    char* parentPath = getParentDirectory(pi->path);

    pathInfo* parentPi = malloc(sizeof(pathInfo));
    parentPi->DEPointer = malloc(sizeof(dirEntry));

    parentPi = parsePath(parentPath);
    dirEntry* tempEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
    loadDirEntries(tempEntries, parentPi->DEPointer->location);

    //Set DE to free state
    tempEntries[pi->value].name[0] = '\0';
    tempEntries[pi->value].dirType = -1;
    releaseFreeSpace(vcb.freeSpaceBitMap, tempEntries[pi->value].location, tempEntries[pi->value].size);
    tempEntries[pi->value].location = -1;
    tempEntries[pi->value].size = 0;
    tempEntries[pi->value].extentLocation = -1;

    updateBitMap(vcb.freeSpaceBitMap);
    tempEntries[0].size -= DE_STRUCT_SIZE;
    //Update .. if its the root directory
    if(tempEntries[0].location == tempEntries[1].location){
        tempEntries[1].size -= DE_STRUCT_SIZE;
    }else{
        char* parentDir = getLastPathElement(cwdPath);
        dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
        LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
        for(int i = 0; i < MAX_DIRENT_SIZE; i++){
            if(strcmp(parentDir, tempDEntries[i].name) == 0){
                tempDEntries[i].size -= DE_STRUCT_SIZE;
                //Exit loop
                i = MAX_DIRENT_SIZE;
            }
        }
        LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
    }
    LBAwrite(tempEntries, DIRECTORY_BLOCKSIZE, tempEntries[0].location);

    return 0;
}

int fs_delete(char* filename){
    pathInfo* pi = malloc(sizeof(pathInfo));
    pi->DEPointer = malloc(sizeof(dirEntry));
    pi = parsePath(filename);
    if(pi->value < 0){
        printf("File doesn't exist\n");
        return -1;
    }
    if(pi->DEPointer->dirType != 0){
        printf("Is not a file\n");
        return -1;
    }
    printf("%d\n",pi->value);
    //Set DE to free state
    cwdEntries[pi->value].name[0] = '\0';
    cwdEntries[pi->value].dirType = -1;
    
    cwdEntries[pi->value].location = -1;
    cwdEntries[pi->value].size = 0;
    releaseFile(cwdEntries[pi->value].extentLocation);
    //printf("Releasing Extent:%d\n ", cwdEntries[pi->value].extentLocation);
    releaseFreeSpace(vcb.freeSpaceBitMap, cwdEntries[pi->value].extentLocation, EXTENT_BLOCK_SIZE);
    cwdEntries[pi->value].extentLocation = -1;

    updateBitMap(vcb.freeSpaceBitMap);
    cwdEntries[0].size -= DE_STRUCT_SIZE;
    //Update .. if its the root directory
    if(cwdEntries[0].location == cwdEntries[1].location){
        cwdEntries[1].size -= DE_STRUCT_SIZE;
    }else{
        //printf("Not supposed to be here!\n");
        char* parentDir = getLastPathElement(cwdPath);
        dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
		LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
        for(int i = 0; i < MAX_DIRENT_SIZE; i++){
            if(strcmp(parentDir, tempDEntries[i].name) == 0){
                tempDEntries[i].size -= DE_STRUCT_SIZE;
                //Exit loop
                i = MAX_DIRENT_SIZE;
            }
        }
        LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);

    }
    LBAwrite(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);
    //Reload cwd
    LBAread(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);
    return 0;

};	

int fs_move(char* src, char* dest){
    pathInfo* srcPi = malloc(sizeof(pathInfo));
    srcPi->DEPointer = malloc(sizeof(dirEntry));
    pathInfo* destPi = malloc(sizeof(pathInfo));
    destPi->DEPointer = malloc(sizeof(dirEntry));
    
    srcPi = parsePath(src);
    destPi = parsePath(dest);

    if(srcPi->value < 0){
        printf("Source File doesn't exist\n");
        return -1;
    }

    if(destPi->value == -2){
        printf("Destination directory doesn't exist\n");
        return -1;
    }
    
    
    //Check the if the destination is a directory
    int isDir = fs_isDir(destPi->path); 
    

    char* oldCwdPath = malloc(strlen(cwdPath));
    strcpy(oldCwdPath, cwdPath);
    printf("%s, %s\n", srcPi->DEPointer->name, destPi->path);
    //Change the path to the destination directory
    if(isDir != 1){
        char* parentDirDest = getParentDirectory(destPi->path);
        fs_setcwd(parentDirDest);
    }else{
        fs_setcwd(destPi->path);
    }
    printf("%s, %s\n", srcPi->DEPointer->name, destPi->DEPointer->name);

    int fileIndex;
    if(destPi->value == -1 || isDir == 1){
        for(int i = 0; i < MAX_DIRENT_SIZE; i++){
            if(cwdEntries[i].dirType == -1){
                fileIndex = i;
                //Exit Loop
                i = MAX_DIRENT_SIZE;
            }
        }
    }else{
        fileIndex = destPi->value;
    }
    
   
    char* destName;

    if(isDir != 1)
        destName = getLastPathElement(destPi->path);
    else
        destName = getLastPathElement(srcPi->path);


    strcpy(cwdEntries[fileIndex].name, destName);
    cwdEntries[fileIndex].dirType = 0;
    cwdEntries[fileIndex].location = srcPi->DEPointer->location;
    cwdEntries[fileIndex].created = srcPi->DEPointer->created;
    time(&cwdEntries[fileIndex].lastModified);
    cwdEntries[fileIndex].size = srcPi->DEPointer->size;
    cwdEntries[fileIndex].extentLocation = srcPi->DEPointer->extentLocation;

    cwdEntries[0].size +=  DE_STRUCT_SIZE;

    

    //Root Directory
    if(cwdEntries[0].location == cwdEntries[1].location){
        cwdEntries[1].size += DE_STRUCT_SIZE;
    }else{
        
        //Update the parent of current directory
        char* parentDir = getLastPathElement(cwdPath);
        dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
		LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
        for(int i = 0; i < MAX_DIRENT_SIZE; i++){
            if(strcmp(parentDir, tempDEntries[i].name) == 0){
                tempDEntries[i].size += DE_STRUCT_SIZE;
                //Exit loop
                i = MAX_DIRENT_SIZE;
            }
        }
        LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
        
    }

    char* parentDirSrc = getParentDirectory(srcPi->path);
    
    fs_setcwd(parentDirSrc);  
    fs_delete(srcPi->path);
    fs_setcwd(oldCwdPath);
    
    free(srcPi->DEPointer);
    free(srcPi);
    free(destPi->DEPointer);
    free(destPi);

    return 0;

}
