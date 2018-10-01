//
// Created by david on 24.09.18.
//

#ifndef VSYS_SERVER_SERVER_H
#define VSYS_SERVER_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <vector>

using namespace std;

class Server {

};


class TCP_Server{
private:
    int serverPort;
    string filePath;
    struct sockaddr_in server_addr;
    int sock_fd;
    bool indicateStop;
public:
    TCP_Server(int port, string path);
    ~TCP_Server();
    void initServer();
    void stopServer();
    void run();
};

void* requestHandler(void *arg);


#endif //VSYS_SERVER_SERVER_H
