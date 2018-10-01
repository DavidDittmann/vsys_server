//
// Created by david on 24.09.18.
//

#include "Server.h"



/*
TCP_Server::TCP_Server(int port, string path){
    this->serverPort = port;
    this->filePath = path;
}
TCP_Server::~TCP_Server(){
    if(this->sock_fd >= 0)
        close(this->sock_fd);
}
void TCP_Server::initServer(){
    if(this->sock_fd < 0)
    {
        this->sock_fd = socket(AF_INET,SOCK_STREAM,0);
        if(this->sock_fd < 0)
        {
            stringstream buf;
            buf << "Socket could not be created!";
            throw runtime_error(buf.str().c_str());
        }
        else
        {
            memset(&this->server_addr,0,sizeof(this->server_addr));
            this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            this->server_addr.sin_family = AF_INET;
            this->server_addr.sin_port = htons(this->serverPort);

            if(bind(this->sock_fd,(struct sockaddr *) &this->server_addr,sizeof(this->server_addr)) == -1)
            {
                stringstream buf;
                buf << "Binding failed!";
                throw runtime_error(buf.str().c_str());
            }
        }
    }
    else
    {
        stringstream buf;
        buf << "Socket already created!";
        throw runtime_error(buf.str().c_str());
    }
}
void TCP_Server::stopServer(){

}
void TCP_Server::run(){
    pthread_t th;
    if(listen(this->sock_fd,5) < 0)
    {
        stringstream buf;
        buf << "Listening failed!";
        throw runtime_error(buf.str().c_str());
    }
    else
    {
        int new_socket = -1;
        int addrlen = sizeof(this->server_addr);
        if((new_socket = accept(this->sock_fd,(struct sockaddr*) &this->server_addr,(socklen_t*)&addrlen)) < 0)
        {
            stringstream buf;
            buf << "Accept failed!";
            throw runtime_error(buf.str().c_str());
        }
        else
        {
            pthread_create(&th,NULL,requestHandler,(void*) &new_socket);
        }
    }

}
void* requestHandler(void *arg){

}
 */