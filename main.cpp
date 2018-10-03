#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>

using namespace std;

void *connection_handler(void *);

int main() {
    signal(SIGPIPE, SIG_IGN);
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket() error");
        return -1;
    }
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(8080);
    sa.sin_addr.s_addr = htons(INADDR_ANY);

    int one = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(sock_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("bind() error");
        return -1;
    }

    if (listen(sock_fd, SOMAXCONN) < 0) {
        perror("listen() error");
        return -1;
    }
    pthread_t thread_id;

    cout << "Will accept..." << endl;
    int conn_fd;
    while((conn_fd = accept(sock_fd, 0, 0)))
    {
        pthread_create(&thread_id, NULL,connection_handler,(void*)&conn_fd);
    }
    if (conn_fd < 0) {
        printf("Fehler bei accept() ...\n");
            exit(EXIT_FAILURE);
    }

    return 0;
}

void *connection_handler(void *socket_desc) {

    int clientfd = *((int *)socket_desc);
    pthread_detach (pthread_self ());

    char buffer[1024];
    memset(buffer,0,sizeof(char)*1024);

    fd_set read_fd;
    FD_ZERO(&read_fd);
    FD_SET(clientfd,&read_fd);
    timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    while(strcmp(buffer,"QUIT")!=0)
    {
        int ret = select(clientfd+1,&read_fd,NULL,NULL,&timeout);
        if(ret > 0)
        {
            if(FD_ISSET(clientfd,&read_fd))
            {
                int ret = recv(clientfd,buffer,1023,0);
                buffer[1023] = '\0';
                cout << buffer << endl;
                ret = send(clientfd, "OK\n", 3,0);
                if(ret < 0)
                {
                    stringstream buf;
                    buf << "Sending msg failed: " << strerror(errno);
                    throw runtime_error(buf.str().c_str());
                }
            }
        }
    }
    close(clientfd);
    return NULL;
}