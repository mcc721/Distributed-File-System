#pragma once

#define SYSTEM_SIZE 100*1024*1024   //100MB
#define BLOCK_SIZE  1024 //1KB
#define BLOCK_COUNT SYSTEM_SIZE/BLOCK_SIZE //100K

class DiskSystem{
public:
    DiskSystem();
    virtual ~DiskSystem();

    void initSystem();
    void exitSystem();

    int getBlock(int blockSize);     
    char* getBlockAddr(int blockNum);
    int getAddrBlock(char *addr);    
    int releaseBlock(int blockNum, int blockSize);

private:
    char* m_systemStartAddr;
};
