#include "client.h"


int main(int argc, char* argv[]) {  
    if (argc<2) {
        help();
        return -1;
    }
    fileOperator fileCmd;
    int fcmd = fileCmd.getOperator(argv[1]); 
    if (fcmd == -1) {
        std::cout << "unknown command: " << argv[1] << std::endl;
        return -1;
    }

    MyChannel channel;
    channel.init("127.0.0.1",1234);
    myserver::FileRequest request;
    myserver::FileResponse response;
    
    request.set_cmd(fcmd);
    if (argc > 2) {
        request.set_filename(argv[2]);
    }
    if (argc > 3) {
        request.set_info(argv[3]);
    }

    myserver::EchoServer_Stub stub(&channel);
    MyController cntl;
    stub.Echo(&cntl, &request, &response, NULL);
    if ( fcmd == 2 ) {
        std::cout << "name\t" << "type" << std::endl;
    }
    for (int i=0; i < response.fileinfo_size(); ++i) {
        if(response.fileinfo(i).name() != "") {
            std::cout << response.fileinfo(i).name() << "\t";
        }
        if(response.fileinfo(i).type() != "") {
            std::cout << response.fileinfo(i).type() << "\t";
        }
        if(response.fileinfo(i).content() != "") {
            std::cout << response.fileinfo(i).content() ;
        }
        std::cout << std::endl;    
    }
    return 0;
}