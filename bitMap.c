#include "bitMap.h"
#include <stdio.h>

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
    return 8-BitCounter(myByte); // gives you the remaining number of free blocks in a byte
};

void setABit(unsigned char *myByte, int offset){
    /*
    setBit(1101 1001, 2) -> 1111 1001
    */
    unsigned char tempByte = mask(offset);
    *myByte = (*myByte | tempByte);
}

void clearABit(unsigned char *myByte, int offset){
    unsigned char tempByte = mask(offset);
    tempByte = ~tempByte;
    *myByte = (*myByte & tempByte);
}

int checkABit(unsigned char myByte, int offset){
    unsigned char tempByte = mask(offset);
    if((myByte & tempByte) == 0x00){
        return 0;
    }else{
        return 1;
    }
}


int checkForConsecFreeSpace(unsigned char myByte, int count){
    int temp = 0;
    if(freeSpaceCounter(myByte) >= count){
        for(int i = 0; i < 8; i++){
            if(checkABit(myByte, i) == 0){
                temp++;
                if(temp == count){
                    return 1;
                }
            }else{
                temp = 0;
            }
        }
    }
    return 0;
}

/*

void initBitMap(){
    char* BitMap = malloc(5*BLOCKSIZE);
    unsigned char tempByte = 0xFC;
    BitMap[0] = 0xFC;
    1111 1100
    for(int i = 1; i < 5*BLOCKSIZE; i++){
        BitMap[i] = 0x00;
    }
    LBAWrite(BitMap, 5, 1);
}
*/

int main(){
    printf("Hello \n");
    unsigned char tempByte = 0x02;
    
    setABit(&tempByte, 2);
    //printf("%d\n", freeSpaceCounter(tempByte));
    //0010 0010
    // clearABit(&tempByte, 6);
    // clearABit(&tempByte, 2);
    printf("%d\n", checkForConsecFreeSpace(tempByte, 2));
    printf("%u\n", tempByte);
    return 0;
}