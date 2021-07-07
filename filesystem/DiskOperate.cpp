#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "DiskOperate.h"

DiskSystem::DiskSystem(){
    try {
        m_systemStartAddr = (char*)malloc(SYSTEM_SIZE * sizeof(char));
        if (m_systemStartAddr == NULL) {
            throw "error: out of memmory.";
        }
    } catch (const char *msg) {
        std::cerr << msg << std::endl;
    }
}

DiskSystem:: ~DiskSystem(){
    free(m_systemStartAddr);
}

//initSystem. Use bitmap to record the usage of memory block. '0'-not used ; '1'-used
void DiskSystem::initSystem() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        m_systemStartAddr[i] = '0';
    }
    int bitMapSize = BLOCK_COUNT * sizeof(char) / BLOCK_SIZE; 
    for (int i = 0; i < bitMapSize; i++) {
        m_systemStartAddr[i] = '1';
    }
}

void DiskSystem::exitSystem() {
    free(m_systemStartAddr);
}

//allocate Block
int DiskSystem::getBlock(int blockSize) {
    int startBlock = 0;
    int sum = 0;
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (m_systemStartAddr[i] == '0') {
            if (sum == 0) {
                startBlock = i;
            }
            sum++;
            if (sum == blockSize) {
                for (int j = startBlock; j < startBlock + blockSize; j++) {
                    m_systemStartAddr[j] = '1';
                }
                return startBlock;
            }
        } else {
            sum = 0;
        }
    }
    return -1;
}

//get the physical address of Block
char* DiskSystem::getBlockAddr(int blockNum) {
    return m_systemStartAddr + blockNum * BLOCK_SIZE; 
}

//find Block idex by physical address
int DiskSystem::getAddrBlock(char* addr) {
    return (addr - m_systemStartAddr) / BLOCK_SIZE;
}

int DiskSystem::releaseBlock(int blockNum, int blockSize) {
    int endBlock = blockNum + blockSize;
    for (int i = blockNum; i < endBlock; i++) {
        m_systemStartAddr[i] = '0';
    }
    return 0;
}