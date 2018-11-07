#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <future>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <map>

using namespace std;

class serverController{
    public:
        serverController(int portArg, string pathArg);
        ~serverController();
        void run();
        //void checkThReady();
    private:
        //vector <future<bool>> vec_future;
        //vector <thread> vec_thread;
        int sock_fd;
        struct sockaddr_in sa;
        int port;
        string path;
        map <string,string> blockedIP;
};

bool loginUser(string user,string pw,int client_fd);

//void connectionHandler(int socket_desc,string path,promise<bool>* promObj);
void connectionHandler(int socket_desc,string path,struct sockaddr_in sa);
void sendData(string msg, int client_fd);
string recvData(int client_fd);
timeval resetSelect(int &clientfd,fd_set &read_fd);
vector<string> split(string strToSplit, char delimeter);
