#pragma once
#include <string>
#include <vector>
#include "DiskOperate.h"

using namespace std;

const int DIRTABLE_MAX_SIZE = 15; //allocate a Block for each dirTable, max_size=15

struct dirUnit {
    char fileName[59];  
    char type;  //'0' - dir , '1' - file
    int startBlock;//the dirTable Block or FCB Block
};

struct dirTable {
    int dirUnitAmount;
    dirUnit dirs[DIRTABLE_MAX_SIZE];
};

struct FCB {
    int fileStartBlock;   //start block number of file
    int fileSize;   //size of file (block)
    int dataSize;   //size of content (byte)
    int readptr;    
    int link;   
};

class FileSystem {
public:
    FileSystem();
    virtual ~FileSystem();
    
    void initRootDir();
    char* getPath();                                //获取当前路径 pwd
    vector<pair<string,string> > getDirTable();     //当前目录列表 ls
    void showDir();                                 //当前目录列表 ls
    int creatDir(string dirName);                   //创建目录 mkdir
    int creatFile(string fileName, int fileSize);   //创建文件 touch
    int changeDir(string dirName);                  //切换目录 cd
    int deleteFile(string fileName);                //删除文件 rm
    int deleteDir(string dirName);                  //删除目录 rmdir
  
    string read(string fileName, int length);       //读文件 read
    string reread(string fileName, int length);     //重新读文件 reread
    int write(string fileName, string content);     //写文件 write
    int rewrite(string fileName, string content);   //重写覆盖 rewrite

    
private:

    dirTable* m_rootDirTable;     //root directory
    dirTable* m_currentDirTable;  //current directory
    char m_curAbsPath[200];       //current absolute path 
    DiskSystem* m_disk;
                                                 
    int findUnitInTable(dirTable* curDirTable, string unitName);    //find dir or file in current directory
    int creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize);    
    int addDirUnit(dirTable* curDirTable, string newDirName, int type, int BlockNum);
    void releaseFile(int FCBBlock);  
    void deleteDirUnit(dirTable* myDirTable, int unitIndex);   
    void deleteFileInTable(dirTable* myDirTable, int unitIndex);   
    string doRead(FCB* myFCB, int length);
    int doWrite(FCB* myFCB, string content);
};
