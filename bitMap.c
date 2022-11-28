#include "bitMap.h"

#include <stdio.h>
#include <stdlib.h>
#include "fsLow.h"

int bitCounter(unsigned char myByte){
    int x = 0;
    if((myByte & 0x80) == 0x80) x++;
    if((myByte & 0x40) == 0x40) x++;
    if((myByte & 0x20) == 0x20) x++;
    if((myByte & 0x10) == 0x10) x++;
    if((myByte & 0x08) == 0x08) x++;
    if((myByte & 0x04) == 0x04) x++;
    if((myByte & 0x02) == 0x02) x++;
    if((myByte & 0x01) == 0x01) x++;
    return x;
} 

unsigned char mask(int offset){
    switch(offset){
        case 0:
            return 0x80;
        case 1:
            return 0x40;
        case 2:
            return 0x20;
        case 3:
            return 0x10;
        case 4:
            return 0x08;
        case 5:
            return 0x04;
        case 6:
            return 0x02;
        case 7: 
            return 0x01;
        default:
            printf("Offset must be between 0-7\n");
            return 0x00;
    }
}

int freeSpaceCounter(unsigned char myByte){
    return 8 - bitCounter(myByte); // gives you the remaining number of free blocks in a byte
};

void setABit(unsigned char *bitMap, int offset){
    /*
    setBit(1101 1001, 2) -> 1111 1001
    */
    int byteIndex = offset/8;
    unsigned char tempByte = mask((offset % 8));
    bitMap[byteIndex] = (bitMap[byteIndex] | tempByte);
}

void clearABit(unsigned char *bitMap, int offset){
    int byteIndex = offset/8;
    unsigned char tempByte = mask((offset % 8));
    tempByte = ~tempByte;
    bitMap[byteIndex] = (bitMap[byteIndex] & tempByte);
}

int checkABit(unsigned char myByte, int offset){
    unsigned char tempByte = mask(offset);
    if((myByte & tempByte) == 0x00){
        return 0;
    }else{
        return 1;
    }
}


int getConsecFreeSpace(unsigned char* bitMap, int bitMapSize, int numOfBlocks){
    int firstFreeBlock;
    int firstFreeByte;
    int minFreeBytesNeeded = (numOfBlocks + 7)/8;
    int freeConsecBytes = 0;

    for(int i =0; i < bitMapSize; i++){
        if(bitMap[i] == 0x00){
            if(freeConsecBytes == 0){
                firstFreeBlock = i*8;
                firstFreeByte = i; 
            }
            freeConsecBytes++;

        }else{
            freeConsecBytes = 0;
        }

        if(freeConsecBytes == minFreeBytesNeeded){
            //Go back a byte and check for extra free space
            for(int j = 7; j >= 0; j--){
                if(checkABit(bitMap[firstFreeByte-1], j) == 0){
                    firstFreeBlock = (firstFreeByte-1)*8 + j;
                }else{
                    //break the for loop
                    j = -1;
                }
            }
            //break the for loop
            i = bitMapSize;
        }
    }

    //Fail to find free consec blocks
    if(freeConsecBytes == 0){
        return -1;
    }

    for(int i = 0; i < numOfBlocks; i++){
        setABit(bitMap, firstFreeBlock + i);
    }

    return firstFreeBlock;
}

int releaseFreeSpace(unsigned char* bitMap, int location, int size){
    
    for(int i = location; i < location+size; i++){
        clearABit(bitMap, i);
        //printf("BitMap is releasing:%d\n ", i);
    }
    return 0;
}

void updateBitMap(unsigned char* bitMap){
	LBAwrite(bitMap, BITMAP_SIZE, BITMAP_LOCATION);
}

// int main(){
//     printf("Hello \n");
//     unsigned char tempByte = 0x02;
//     unsigned char* bitMap = malloc(5);

//     bitMap[0] = 0xFC;
//     for(int i = 1; i<5; i++){
//         bitMap[i] = 0x00;
//     }
//     bitMap[2] = 0xFF;

//     //1111 1100 0000 0000 1111 1111 0000 0000 0000 0000

//     //printf("%d\n", freeSpaceCounter(tempByte));
//     //0010 0010
//     // clearABit(&tempByte, 6);
//     // clearABit(&tempByte, 2);
//     printf("%d\n", getConsecFreeSpace(bitMap, 5, 6));
//     return 0;
// }