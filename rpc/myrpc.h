#include <iostream>
#include "rpc_meta.pb.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/service.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/message.h"
#include "boost/asio.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"

//MyController
class MyController : public ::google::protobuf::RpcController {
public:
    MyController() { Reset(); }
    virtual ~MyController(){}
  
    virtual void Reset() { m_isFailed = false; m_errorCode = "";};

    virtual bool Failed() const { return m_isFailed; };
    virtual void SetFailed(const std::string& reason) { 
        m_isFailed = true;
        m_errorCode = reason;
    };
  virtual std::string ErrorText() const { return m_errorCode; };
  virtual void StartCancel() { };

  virtual bool IsCanceled() const { return false; };
  virtual void NotifyOnCancel(::google::protobuf::Closure* /* callback */) { };

private:
  bool m_isFailed;
  std::string m_errorCode;
};

//mychannel
class MyChannel : public ::google::protobuf::RpcChannel {
public:
    void init(const std::string& ip, const int port) {
        _io = boost::make_shared<boost::asio::io_service>();
        _sock = boost::make_shared<boost::asio::ip::tcp::socket>(*_io);
        boost::asio::ip::tcp::endpoint ep(
                boost::asio::ip::address::from_string(ip), port);
        _sock->connect(ep);
    }

    virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
            ::google::protobuf::RpcController* controller ,
            const ::google::protobuf::Message* request,
            ::google::protobuf::Message* response,
            ::google::protobuf::Closure* done) {
        std::string request_data_str = request->SerializeAsString();

        //send format: meta_size + meta_data + request_data
        myrpc::RpcMeta rpc_meta;
        rpc_meta.set_service_name(method->service()->name());
        rpc_meta.set_method_name(method->name());
        rpc_meta.set_data_size(request_data_str.size());

        std::string rpc_meta_str = rpc_meta.SerializeAsString();
        int rpc_meta_str_size = rpc_meta_str.size();
        rpc_meta_str.insert(0, std::string((const char*)&rpc_meta_str_size, sizeof(int)));
        rpc_meta_str += request_data_str;
        _sock->send(boost::asio::buffer(rpc_meta_str));

        //receive format: respone_size + response_data
        char resp_data_size[sizeof(int)];
        _sock->receive(boost::asio::buffer(resp_data_size));

        int resp_data_len = *(int*)resp_data_size;
        std::vector<char> resp_data(resp_data_len, 0);
        _sock->receive(boost::asio::buffer(resp_data));

        response->ParseFromString(std::string(&resp_data[0], resp_data.size()));
    }

private:
    boost::shared_ptr<boost::asio::io_service> _io;
    boost::shared_ptr<boost::asio::ip::tcp::socket> _sock;
};

//MyServer
class MyRpcServer {
public:
    MyRpcServer(){}
    virtual ~MyRpcServer(){}
    
    void RegisterService(::google::protobuf::Service* service);
    void Start(const std::string& ip, const int port);

private:
    void ProcRpcData(
            const std::string& service_name,
            const std::string& method_name,
            const std::string& serialzied_data,
            const boost::shared_ptr<boost::asio::ip::tcp::socket>& sock);
    void OnCallbackDone(
            ::google::protobuf::Message* resp_msg,
            const boost::shared_ptr<boost::asio::ip::tcp::socket> sock);

private:
    struct ServiceInfo{
        ::google::protobuf::Service* service;
        const ::google::protobuf::ServiceDescriptor* sd;
        std::map<std::string, const ::google::protobuf::MethodDescriptor*> mds;
    };//ServiceInfo

    std::map<std::string, ServiceInfo> _services;
};

void MyRpcServer::RegisterService(::google::protobuf::Service* service) {      
    ServiceInfo service_info;
    service_info.service = service;
    service_info.sd = service->GetDescriptor();
    for (int i = 0; i < service_info.sd->method_count(); ++i) {
        service_info.mds[service_info.sd->method(i)->name()] = service_info.sd->method(i);
    }
        _services[service_info.sd->name()] = service_info;
}

void MyRpcServer::Start(const std::string& ip, const int port) {
    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acceptor(io,
            boost::asio::ip::tcp::endpoint(
                boost::asio::ip::address::from_string(ip), port));

    while (true) {
        auto sock = boost::make_shared<boost::asio::ip::tcp::socket>(io);
        acceptor.accept(*sock);

        std::cout << "recv from client:"
            << sock->remote_endpoint().address()
            << std::endl;

        //receive meta_size
        char meta_size[sizeof(int)]; //consistent with the type of "send"
        sock->receive(boost::asio::buffer(meta_size));
        int meta_len = *(int*)(meta_size);

        //receive meta_data
        std::vector<char> meta_data(meta_len, 0);
        sock->receive(boost::asio::buffer(meta_data));
        myrpc::RpcMeta meta;
        meta.ParseFromString(std::string(&meta_data[0], meta_data.size()));

        //receive request_data
        std::vector<char> data(meta.data_size(), 0);
        sock->receive(boost::asio::buffer(data));

        ProcRpcData(
                meta.service_name(),
                meta.method_name(),
                std::string(&data[0], data.size()),
                sock);
    }
}

//processing RpcData
void MyRpcServer::ProcRpcData(
        const std::string& service_name,
        const std::string& method_name,
        const std::string& serialzied_data,
        const boost::shared_ptr<boost::asio::ip::tcp::socket>& sock) {
    auto service = _services[service_name].service;
    auto md = _services[service_name].mds[method_name];
    /*
    std::cout << "recv service_name:" << service_name << std::endl;
    std::cout << "recv method_name:" << method_name << std::endl;
    std::cout << "recv type:" << md->input_type()->name() << std::endl;
    std::cout << "resp type:" << md->output_type()->name() << std::endl;
    */
    auto recv_msg = service->GetRequestPrototype(md).New();
    recv_msg->ParseFromString(serialzied_data);
    auto resp_msg = service->GetResponsePrototype(md).New();

    MyController controller;
    auto done = ::google::protobuf::NewCallback(
            this,
            &MyRpcServer::OnCallbackDone,
            resp_msg,
            sock);
    service->CallMethod(md, &controller, recv_msg, resp_msg, done);
}

void MyRpcServer::OnCallbackDone(
        ::google::protobuf::Message* resp_msg,
        const boost::shared_ptr<boost::asio::ip::tcp::socket> sock) {
    
    int serialized_size = resp_msg->ByteSize();
    std::string resp_data;
    resp_data.insert(0, std::string((const char*)&serialized_size, sizeof(int)));
    resp_msg->AppendToString(&resp_data);

    sock->send(boost::asio::buffer(resp_data));
}
