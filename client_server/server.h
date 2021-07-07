#pragma once
#include "myrpc.h"
#include "myserver.pb.h"
#include "flag.h"
#include "DiskOperate.h"
#include "FileOperate.h"

class MyServiceImpl : public myserver::EchoServer {
public:
    MyServiceImpl(){
        m_filesystem = new FileSystem();
        m_filesystem->initRootDir();
    }
    virtual ~MyServiceImpl(){}

private:
    void Echo(::google::protobuf::RpcController*  controller,
                            const ::myserver::FileRequest* request,
                           ::myserver::FileResponse* response,
                           ::google::protobuf::Closure* done) {
        std::string fname = request->filename();
        std::string info = request->info();
        switch (request->cmd())
        {
        case PWD: {
            std::cout << "recv cmd: pwd" << std::endl;
            m_filesystem->showDir();
            _fileinfo = response->add_fileinfo();
            std::string str = m_filesystem->getPath();
            _fileinfo->set_content(str);
            break;
        }
        case LS: {
            std::cout << "recv cmd: ls" << std::endl;
            vector<pair<string, string> > dirInfo = m_filesystem->getDirTable();
            for (auto it : dirInfo){
                _fileinfo = response->add_fileinfo();
                _fileinfo->set_name(it.first);
                _fileinfo->set_type(it.second);
            }
            break; 
        }
        case MKDIR: {
            std::cout << "recv cmd: mkdir " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if( m_filesystem->creatDir(fname) == -1) {
                _fileinfo->set_content("create " + fname + " fail.");
            } else {
                _fileinfo->set_content("create " + fname + " succeed.");
            }
            break;
        }
        case TOUCH: {
            std::cout << "recv cmd: touch " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if (m_filesystem->creatFile(fname, atoi(info.c_str())) == -1) {
                _fileinfo->set_content("create " + fname + " fail.");
            } else {
                _fileinfo->set_content("create " + fname + " succeed.");
            }
            break;
        }
        case CD: {
            std::cout << "recv cmd: cd " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if (m_filesystem->changeDir(fname) == -1) {
                _fileinfo->set_content("cd " + fname + " fail.");
            } else {
                _fileinfo->set_content("cd " + fname + " succeed.");
            }
            break;
        }
        case RM: {
            std::cout << "recv cmd: rm " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if (m_filesystem->deleteFile(fname) == -1) {
                _fileinfo->set_content("rm " + fname + " fail.");
            } else {
                _fileinfo->set_content("rm " + fname + " succeed.");
            }
            break;
        }
        case RMDIR: {
            std::cout << "recv cmd: rmdir " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if (m_filesystem->deleteDir(fname) == -1) {
                _fileinfo->set_content("rmdir " + fname + " fail.");
            } else {
                _fileinfo->set_content("rmdir " + fname + " succeed.");
            }
            break;
        }
        case READ: {
            std::cout << "recv cmd: read " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            std::string str = m_filesystem->read(fname,atoi(info.c_str()));
            _fileinfo->set_content(str);
            break;
        }
        case REREAD: {
            std::cout << "recv cmd: reRead " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            std::string str = m_filesystem->reread(fname,atoi(info.c_str()));
            _fileinfo->set_content(str);
            break;
        }
        case WRITE:{
            std::cout << "recv cmd: write " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if (m_filesystem->write(fname,info) == -1) {
                _fileinfo->set_content("write " + fname + " fail.");
            } else {
                _fileinfo->set_content("write " + fname + " succeed.");
            }
            break;
        }
        case REWRITE:{
            std::cout << "recv cmd: reWrite " << request->filename() << std::endl;
            _fileinfo = response->add_fileinfo();
            if (m_filesystem->write(fname,info) == -1) {
                _fileinfo->set_content("rewrite " + fname + " fail.");
            } else {
                _fileinfo->set_content("rewrite " + fname + " succeed.");
            }
            break;
        }
        default:
            break;
        }
        done->Run();
   }

private:
    ::myserver::FileInfo* _fileinfo;
    FileSystem* m_filesystem;
};
