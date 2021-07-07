# A Simple Distributed File System
## Requirements
### a.protobuf
```
git clone https://github.com/protocolbuffers/protobuf.git
sudo apt-get install autoconf  automake  libtool curl make  g++  unzip libffi-dev -y
cd protobuf
./autogen.sh
./configure
make
sudo make install
sudo ldconfig
```

### b.boost
```
apt-get install mpi-default-dev
sudo apt-get install mpi-default-dev
sudo apt-get install libicu-dev
sudo apt-get install libbz2-dev
wget https://raw.githubusercontent.com/lylei9/boost_1_57_0/master/boost_1_57_0.tar.gz
tar zxvf boost_1_57_0.tar.gz
cd boost_1_57_0
./bootstrap.sh
sudo ./b2
sudo ./b2 install
```

## Usage
### compile *.proto
`protoc -I=. --cpp_out=. *.proto`
### build
```
cmake .
make
```
### test
`./server`

`./client`