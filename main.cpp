#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <thread>
#include <future>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <vector>

using namespace std;

void connection_handler(int socket_desc,promise<bool>* promObj);

int main() {
    signal(SIGPIPE, SIG_IGN);
    vector <future<bool>> vec_future;
    vector <thread> vec_thread;
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

    cout << "Will accept..." << endl;
    int conn_fd;
    while(true)
    {
        conn_fd = accept(sock_fd, 0, 0);
        if (conn_fd < 0) {
            printf("Fehler bei accept() ...\n");
            exit(EXIT_FAILURE);
        } else
        {
            promise<bool> promObj;
            future<bool> futObj = promObj.get_future();
            thread th(connection_handler,conn_fd,&promObj);
            vec_future.push_back(futObj);
            vec_thread.push_back(th);
        }
        int vecSize = vec_future.size();
        for(int i=0;i<vecSize;i++)
        {
            auto status = vec_future[i].wait_for(0ms);
            if (status == std::future_status::ready) {
                vec_thread[i].join;
                vec_thread[i].delete();
                vec_future[i].delete();
                vecSize--;
            }
            else
                i++;

        }
    }


    return 0;
}

void connection_handler(int socket_desc,promise<bool>* promObj) {

    int clientfd = socket_desc;

    string User = "ERROR_USER";
    vector<char*> MSG;

    char buffer[1024];
    memset(buffer,0,sizeof(char)*1024);

    fd_set read_fd;
    FD_ZERO(&read_fd);
    FD_SET(clientfd,&read_fd);
    timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    while(strcmp(buffer,"QUIT\n")!=0)
    {
        MSG.clear();
        int ret = select(clientfd+1,&read_fd,NULL,NULL,&timeout);
        if(ret > 0)
        {
            if(FD_ISSET(clientfd,&read_fd))
            {
                int ret = recv(clientfd,buffer,1023,0);
                buffer[1023] = '\0';
                cout << buffer << endl;

                char* chars_array = strtok(buffer, "\n");
                while(chars_array)
                {
                    MSG.push_back(chars_array);
                    chars_array = strtok(NULL, "\n");
                }
                if(strcmp(MSG[0],"QUIT")==0)
                {

                }
            }
        }
    }
    close(clientfd);
}

bool loginLDAP(string user,string pw)
{

}