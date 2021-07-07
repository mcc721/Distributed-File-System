cmake_minimum_required(VERSION 3.9)
project(MyFileSystem)
add_compile_options(-std=c++11)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(./rpc
                    ./filesystem
                    ./client_server
                    ${Boost_INCLUDE_DIRS}
                    ${PROTOBUF_INCLUDE_DIRS})

set(LIBMYRPC_SRC ./rpc/rpc_meta.pb.cc)
add_library(myrpc ${LIBMYRPC_SRC})

add_executable(server ./client_server/server.cpp
                      ./client_server/myserver.pb.cc
                      ./filesystem/DiskOperate.cpp
                      ./filesystem/FileOperate.cpp)
target_link_libraries(server libprotobuf.a 
                             libboost_signals.a 
                             libboost_system.a 
                             pthread 
                             myrpc)
add_executable(client ./client_server/client.cpp
                      ./client_server/myserver.pb.cc)
target_link_libraries(client libprotobuf.a 
                             libboost_signals.a 
                             libboost_system.a 
                             pthread 
                             myrpc)
                      

