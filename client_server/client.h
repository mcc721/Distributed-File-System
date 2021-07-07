#pragma once
#include <iostream>
#include <map>
#include "myrpc.h"
#include "myserver.pb.h"


void help() {
    std::cout << "Use command:" << std::endl;
    std::cout << "pwd\t\t\t :print work directory " << std::endl;
    std::cout << "ls\t\t\t :list files" << std::endl;
    std::cout << "mkdir dirName\t\t :make directory" << std::endl;
    std::cout << "touch fileName size\t :creat file" << std::endl;
    std::cout << "cd dirName\t\t :change directory" << std::endl;
    std::cout << "rm fileName\t\t :remove file" << std::endl;
    std::cout << "rmdir dirName\t\t :remove directory" << std::endl;
    std::cout << "read fileName size\t :read file" << std::endl;
    std::cout << "reread fileName size\t :reread file" << std::endl;
    std::cout << "write fileName content\t :write file" << std::endl;
    std::cout << "rewrite fileName content:rewrite file" << std::endl;
}

class fileOperator{
public:
    fileOperator(){
        m_operator.insert(std::pair<std::string, int>("pwd", 1));
        m_operator.insert(std::pair<std::string, int>("ls", 2));
        m_operator.insert(std::pair<std::string, int>("mkdir", 3));
        m_operator.insert(std::pair<std::string, int>("touch", 4));
        m_operator.insert(std::pair<std::string, int>("cd", 5));
        m_operator.insert(std::pair<std::string, int>("rm", 6));
        m_operator.insert(std::pair<std::string, int>("rmdir", 7));
        m_operator.insert(std::pair<std::string, int>("read", 8));
        m_operator.insert(std::pair<std::string, int>("reread", 9));
        m_operator.insert(std::pair<std::string, int>("write", 10));
        m_operator.insert(std::pair<std::string, int>("rewrite", 11));
    }
    virtual ~fileOperator(){}

    int getOperator(std::string op){
        std::map<std::string,int>::iterator iter = m_operator.find(op);
        if(iter == m_operator.end()) {
            return -1;
        }
        return iter->second;
    }

private:
    std::map<std::string,int> m_operator;
};
