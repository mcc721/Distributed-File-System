#include <iostream>
#include <stdio.h>
#include <string>
//#include <string.h>
#include <cstring>
#include "FileOperate.h"


FileSystem::FileSystem() {
    m_disk=new DiskSystem();
    m_disk->initSystem();
}

FileSystem::~FileSystem() {
}

void FileSystem::initRootDir() {
    int startBlock = m_disk->getBlock(1);
    if (startBlock == -1) {
        return;
    }
    m_rootDirTable = (dirTable*)m_disk->getBlockAddr(startBlock);
    m_rootDirTable->dirUnitAmount = 0;
    addDirUnit(m_rootDirTable, "..", 0, startBlock);

    m_currentDirTable = m_rootDirTable;
    m_curAbsPath[0] = '/';
    m_curAbsPath[1] = '\0';
}

//command: pwd
char* FileSystem::getPath() {
    return m_curAbsPath;
}

//command: ls
void FileSystem::showDir() {
    int unitAmount = m_currentDirTable->dirUnitAmount;
    std::cout << "total: "<< unitAmount-1 <<std::endl;
    std::cout << "name\ttyple" << std::endl;
    for (int i = 1; i < unitAmount; ++i) {
        dirUnit unitTemp = m_currentDirTable->dirs[i];
        std::cout << unitTemp.fileName << "\t"
        << (unitTemp.type == 0 ? "dir" : "file") << std::endl;
    }
}

vector<pair<string,string> > FileSystem::getDirTable() {
    vector<pair<string,string> > _dirtable;
    int unitAmount = m_currentDirTable->dirUnitAmount;
    for (int i = 1; i < unitAmount; ++i) {
        pair<string, string> temp;
        dirUnit unitTemp = m_currentDirTable->dirs[i];
        temp.first = unitTemp.fileName;
        temp.second =  (unitTemp.type == 0 ? "dir" : "file");
        _dirtable.push_back(temp);
    }
    return _dirtable;
}

//command: mkdir
int FileSystem::creatDir(string dirName) {
    if (dirName.length() >= 59) {
        std::cout << "file name is too long" << std::endl;
        return -1;
    }
    //allocate a Block to new dirTable 
    int dirBlock = m_disk->getBlock(1);
    if (dirBlock == -1) {
        std::cout << "creat fail: memory is full." << std::endl;
        return -1;
    }
    if (addDirUnit(m_currentDirTable, dirName, 0, dirBlock) == -1) {
        m_disk->releaseBlock(dirBlock, 1);
        return -1;
    }
    dirTable* newTable = (dirTable*)m_disk->getBlockAddr(dirBlock);
    newTable->dirUnitAmount = 0;
    char parent[] = "..";
    addDirUnit(newTable, parent, 0, m_disk->getAddrBlock((char*)m_currentDirTable));
    return 0;
}

//command: touch
int FileSystem::creatFile(string fileName, int fileSize) {
    if (fileName.length() >= 59) {
        std::cout << "file name is too long" << std::endl;
        return -1;
    }
    //allocate a Block to FCB
    int FCBBlock = m_disk->getBlock(1);
    if (FCBBlock == -1) {
        std::cout << "creat fail: memory is full." << std::endl;
        return -1;
    }
    int FileBlock = m_disk->getBlock(fileSize);
    if (FileBlock == -1) {
        std::cout << "creat fail: memory is full." << std::endl;
        return -1;
    }
    creatFCB(FCBBlock, FileBlock, fileSize);
    if (addDirUnit(m_currentDirTable, fileName, 1, FCBBlock) == -1) {
        m_disk->releaseBlock(FCBBlock, 1);
        m_disk->releaseBlock(FileBlock, fileSize);
        return -1;
    }
    return 0;
}

//command: cd
int FileSystem::changeDir(string dirName) {
    int unitIndex = findUnitInTable(m_currentDirTable, dirName);
    if (unitIndex == -1) {
        std::cout << "file not found" << std::endl;
        return -1;
    }
    if (m_currentDirTable->dirs[unitIndex].type == 1) {
        std::cout << "not a dir" << std::endl;
        return -1;
    }
    int dirBlockNum = m_currentDirTable->dirs[unitIndex].startBlock;
    m_currentDirTable = (dirTable*)m_disk->getBlockAddr(dirBlockNum);
    if (strcmp(dirName.c_str(), "..") == 0) {
        int len = strlen(m_curAbsPath);
        for (int i = len - 2; i >= 0; i--)
            if (m_curAbsPath[i] == '/') {
                m_curAbsPath[i + 1] = '\0';
                break;
            }
    } else {
        strcat(m_curAbsPath, dirName.c_str());
        strcat(m_curAbsPath, "/");
    }
    return 0;
}

//command: rm (remove file)
int FileSystem::deleteFile(string fileName) {
    if (strcmp(fileName.c_str(), "..") == 0) {
        std::cout << "can't delete .." << std::endl;
        return -1;
    }
    int unitIndex = findUnitInTable(m_currentDirTable, fileName);
    if (unitIndex == -1) {
        std::cout << "file not found" << std::endl;
        return -1;
    }

    dirUnit _Unit = m_currentDirTable->dirs[unitIndex];
    if (_Unit.type == 0) {
        std::cout << "not a file" << std::endl;
        return -1;
    }
    int FCBBlock = _Unit.startBlock;
    //release the memmory of the file and FCB
    releaseFile(FCBBlock);
    //delete the record from pre_dirTable
    deleteDirUnit(m_currentDirTable, unitIndex);
    std::cout << "delete " << fileName << " succeeded" << std::endl;
    return 0;
}

//command: rmdir
int FileSystem::deleteDir(string dirName) {
    if (strcmp(dirName.c_str(), "..") == 0) {
        std::cout << "can't delete .." << std::endl;
        return -1;
    }
    int unitIndex = findUnitInTable(m_currentDirTable, dirName);
    if (unitIndex == -1) {
        std::cout << "file not found" << std::endl;
        return -1;
    }
    dirUnit _Unit = m_currentDirTable->dirs[unitIndex];

    //delete dir
    if (_Unit.type == 0) {
        deleteFileInTable(m_currentDirTable, unitIndex);
    } else {
        std::cout << "not a dir" << std::endl;
        return -1;
    }
    //delete the record from pre_dirTable
    deleteDirUnit(m_currentDirTable, unitIndex);
    std::cout << "delete " << dirName << " succeeded" << std::endl;
    return 0;
}

int FileSystem::creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize)
{
    FCB* currentFCB = (FCB*)m_disk->getBlockAddr(fcbBlockNum);
    currentFCB->fileStartBlock = fileBlockNum;//the idex of the start fileBlock
    currentFCB->fileSize = fileSize;
    currentFCB->link = 1;
    currentFCB->dataSize = 0;
    currentFCB->readptr = 0;
    return 0;
}

int FileSystem::addDirUnit(dirTable* curDirTable, string newDirName, int type, int BlockNum) {

    int dirUnitAmount = curDirTable->dirUnitAmount;
    if (dirUnitAmount == DIRTABLE_MAX_SIZE) {
        std::cout << "creat fail: dirTables is full, try to delete some file." << std::endl;
        return -1;
    }
    if (findUnitInTable(curDirTable, newDirName) != -1) {
        std::cout << "file already exist" << std::endl;
        return -1;
    }
    
    dirUnit* newDirUnit = &curDirTable->dirs[dirUnitAmount];
    curDirTable->dirUnitAmount++;
    strcpy(newDirUnit->fileName, newDirName.c_str());
    newDirUnit->type = type;
    newDirUnit->startBlock = BlockNum;

    return 0;
}

//release a file by FCBBlock
void FileSystem::releaseFile(int FCBBlock) {
    FCB* myFCB = (FCB*)m_disk->getBlockAddr(FCBBlock);
    myFCB->link--;
    //release file block  
    if (myFCB->link == 0) {
        m_disk->releaseBlock(myFCB->fileStartBlock, myFCB->fileSize);
    }
    //release FCBBlock
    m_disk->releaseBlock(FCBBlock, 1);
}

//delete a dirUint from pre_dirTable
void FileSystem::deleteDirUnit(dirTable* preDirTable, int unitIndex) {
    int dirUnitAmount = preDirTable->dirUnitAmount;
    for (int i = unitIndex; i < dirUnitAmount - 1; i++) {
        preDirTable->dirs[i] = preDirTable->dirs[i + 1];
    }
    preDirTable->dirUnitAmount--;
}


//delete file or dir
void FileSystem::deleteFileInTable(dirTable* myDirTable, int unitIndex) {

    dirUnit _Unit = myDirTable->dirs[unitIndex];

    if (_Unit.type == 0) { //delete dir
        int unitBlock = _Unit.startBlock;
        dirTable* table = (dirTable*)m_disk->getBlockAddr(unitBlock);
        int unitCount = table->dirUnitAmount;
        for (int i = 1; i < unitCount; i++) {
            deleteFileInTable(table, i);
        }
        m_disk->releaseBlock(unitBlock, 1);
    } else {//delete file
        int FCBBlock = _Unit.startBlock;
        releaseFile(FCBBlock);
    }
}



/**********************Read and Write operate*******************/
//Read file
string FileSystem::read(string fileName, int length) {
    int unitIndex = findUnitInTable(m_currentDirTable, fileName);
    if (unitIndex == -1) {
        std::cout << "file no found" << std::endl;
        return "";
    }
    if (m_currentDirTable->dirs[unitIndex].type == 0) {
        std::cout << "Read fail: "
                  << fileName << " is not a file" << std::endl;
        return "";
    }
    int FCBBlock = m_currentDirTable->dirs[unitIndex].startBlock;
    FCB* myFCB = (FCB*)m_disk->getBlockAddr(FCBBlock);
    string str = doRead(myFCB, length);
    return str;
}

//reRead file
string FileSystem::reread(string fileName, int length) {
    int unitIndex = findUnitInTable(m_currentDirTable, fileName);
    if (unitIndex == -1) {
        std::cout << "file no found" << std::endl;
        return "";
    }
    if (m_currentDirTable->dirs[unitIndex].type == 0) {
        std::cout << "reRead fail: "
                  << fileName << " is not a file" << std::endl;
        return "";
    }
    int FCBBlock = m_currentDirTable->dirs[unitIndex].startBlock;
    FCB* myFCB = (FCB*)m_disk->getBlockAddr(FCBBlock);
    myFCB->readptr = 0;
    string str = doRead(myFCB, length);
    return str;
}

string FileSystem::doRead(FCB* myFCB, int length) {
    int dataSize = myFCB->dataSize;
    char* data = (char*)m_disk->getBlockAddr(myFCB->fileStartBlock);
    string str;
    //read n byte from file, n = length
    for (int i = 0; i < length && myFCB->readptr < dataSize; i++, myFCB->readptr++) {
        str += *((data + myFCB->readptr));
    }
    return str;
}

//Write file
int FileSystem::write(string fileName, string content) {
    int unitIndex = findUnitInTable(m_currentDirTable, fileName);
    if (unitIndex == -1) {
        std::cout << "file no found" << std::endl;
        return -1;
    }
    if (m_currentDirTable->dirs[unitIndex].type == 0) {
        std::cout << "Write fail: "
                  << fileName << " is not a file" << std::endl;
        return -1;
    }
    int FCBBlock = m_currentDirTable->dirs[unitIndex].startBlock;
    FCB* myFCB = (FCB*)m_disk->getBlockAddr(FCBBlock);
    doWrite(myFCB, content);
    return 0;
}

//reWrite file
int FileSystem::rewrite(string fileName, string content) {
    int unitIndex = findUnitInTable(m_currentDirTable, fileName);
    if (unitIndex == -1) {
        std::cout << "file no found" << std::endl;
        return -1;
    }
    if (m_currentDirTable->dirs[unitIndex].type == 0) {
        std::cout << "Write fail: "
                  << fileName << " is not a file" << std::endl;
        return -1;
    }
    int FCBBlock = m_currentDirTable->dirs[unitIndex].startBlock;
    FCB* myFCB = (FCB*)m_disk->getBlockAddr(FCBBlock);
    //clean file content
    myFCB->dataSize = 0;
    myFCB->readptr = 0;
    doWrite(myFCB, content);
    return 0;
}

int FileSystem::doWrite(FCB* myFCB, string content) {
    int contentLen = content.length();
    int fileSize = myFCB->fileSize * BLOCK_SIZE;
    char* data = (char*)m_disk->getBlockAddr(myFCB->fileStartBlock);
    for (int i = 0; i < contentLen && myFCB->dataSize < fileSize; i++, myFCB->dataSize++) {
        *(data + myFCB->dataSize) = content[i];
    }
    if (myFCB->dataSize == fileSize) {
        std::cout << "file is full" << std::endl;
    }
    return 0;
}

int FileSystem::findUnitInTable(dirTable* curDirTable, string unitName) {
    int dirUnitAmount = curDirTable->dirUnitAmount;
    int unitIndex = -1;
    for (int i = 0; i < dirUnitAmount; i++)
        if (strcmp(unitName.c_str(), curDirTable->dirs[i].fileName) == 0)
            unitIndex = i;
    return unitIndex;   
}
