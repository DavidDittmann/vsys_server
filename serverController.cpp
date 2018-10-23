#include "serverController.h"
#include "ldapController.h"
#include "fileController.h"

serverController::serverController(int portArg, string pathArg)
{
    this->port = portArg;
    this->path = pathArg;

    this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket() error");
    }
    this->sa.sin_family = AF_INET;
    this->sa.sin_port = htons(this->port);
    this->sa.sin_addr.s_addr = htons(INADDR_ANY);

    int one = 1;
    setsockopt(this->sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(this->sock_fd, (struct sockaddr*)&this->sa, sizeof(this->sa)) < 0) {
        perror("bind() error");
    }

    if (listen(this->sock_fd, SOMAXCONN) < 0) {
        perror("listen() error");
    }
}

serverController::~serverController()
{
    if(this->sock_fd>0)
        close(sock_fd);

    cout << "Waiting for threads to finish..." << endl;
    int size = this->vec_thread.size();
    for(int i=0;i<size;i++)
        vec_thread[i].join();
    cout << "Threads finished." << endl;

    this->vec_future.clear();
    this->vec_thread.clear();
}

void serverController::run()
{
    cout << "Running..." << endl;
    int conn_fd;
    while(true)
    {
        conn_fd = accept(this->sock_fd,0,0);
        if (conn_fd < 0) {
            printf("Fehler bei accept() ...\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            //IMPLEMENT CHECK IF BANNED

            promise<bool> promObj;
            future<bool> futObj = promObj.get_future();
            string pathD = this->path;
            thread th(connectionHandler,conn_fd,pathD,&promObj);
            this->vec_future.push_back(move(futObj));
            this->vec_thread.push_back(move(th));
        }
        checkThReady();
    }
}

void serverController::checkThReady()
{
    int vecSize = this->vec_future.size();
    for(int i=0;i<vecSize;i++)
    {
        auto status = this->vec_future[i].wait_for(chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            this->vec_thread[i].join();

            this->vec_thread.erase((vec_thread.begin()+i));
            this->vec_future.erase((vec_future.begin()+i));
            vecSize--;
        }
        else
            i++;

    }
}

bool loginUser(string user, string pw,int client_fd)
{
    sendData("OK\n", client_fd);
    return true;    //FOR DEBUGING ONLY
    /*
    if(loginLDAP(user,pw,client_fd))
    {
        sendData("OK\n",client_fd);
        return true;
    }
    else
    {
        sendData("ERR\n",client_fd);
        return false;
    }*/
}


void connectionHandler(int socket_desc,string path,promise<bool>* promObj)
{
    cout << "connected..." << endl;
    int clientfd = socket_desc;
    string User = "ERROR_USER";
    int failedLogins = 0;

    fd_set read_fd;
    string msg = "";
    char delim = '\n';
    vector<string>msg_vec;
    string order = "";
    string option1="",option2="",option3="";

    while(order!="QUIT")
    {
        if(failedLogins>2)
        {
            //TO IMPLEMENT BAN IP !!!!
            order = "QUIT";
        }
        else{
            timeval timeout = resetSelect(clientfd,read_fd);

            int ret = select(clientfd+1,&read_fd,NULL,NULL,&timeout);
            if(ret > 0)
            {
                if(FD_ISSET(clientfd,&read_fd))
                {

                    msg = recvData(clientfd);
                    msg_vec = split(msg,delim);
                    msg_vec.pop_back();
                    if(!msg_vec.empty())
                    {
                        cout << "User: " << User << endl;
                        order = msg_vec[0];
                        cout << "msg: " <<  msg << endl;
                        cout << "msg_vec_size: " << msg_vec.size()<<endl;
                        cout << "MSGBACK: " << msg_vec.back() << endl;
                        cout << "Order: " << order << "|" << msg_vec.size() << endl;

                        // **************** LOGIN *********************
                        if(order == "LOGIN" && msg_vec.size()==3)
                        {
                            cout << "login try..." <<endl;
                            if(loginUser(msg_vec[1],msg_vec[2],clientfd))
                            {    //sendData("OK\n",clientfd);
                                cout << "login successed" <<endl;
                                failedLogins = 0;
                                User = msg_vec[1];
                            }
                            else
                            {
                                failedLogins++;
                                cout << "failed login: " << failedLogins << endl;
                            }

                        }
                        // ******************** READ ***********************
                        else if(order == "READ" && msg_vec.size()==3)
                        {
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else
                            {
                                int mailToRead = stoi(msg_vec[2]);
                                string res = readMail(path,User,mailToRead);
                                sendData(res,clientfd);
                            }
                        }
                        //********************** SEND *********************
                        else if(order == "SEND" && msg_vec.size()>=6 && msg_vec.back() == ".")
                        {
                            cout << "User: " << User << endl;
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else
                            {
                                string to = msg_vec[2];
                                string subj = msg_vec[3];
                                int i = 4; string nachricht="";
                                while(i<msg_vec.size()-1)
                                {
                                    nachricht += msg_vec[i] +"\n";
                                    i++;
                                }

                                //Speichern der Nachricht
                                cout << "saving msg" <<endl;
                                if(safeMsg(path,User,to,subj,nachricht))
                                {
                                    sendData("OK\n",clientfd);
                                }
                                else
                                {
                                    sendData("ERR\n",clientfd);
                                }
                            }
                        }
                        //************************* LIST ********************
                        else if(order == "LIST" && msg_vec.size()==2)
                        {
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else
                            {
                                vector <string> mailList = listMails(path,User);
                                if(!mailList.empty()){
                                    string res = to_string(mailList.size())+"\n";
                                    cout << "MAILLIST:" <<endl;
                                    for(auto iter : mailList)
                                    {
                                        res += iter + "\n";
                                        cout << iter <<endl;
                                    }
                                    sendData(res,clientfd);
                                }
                                else{
                                    string res = "0\n";
                                    sendData(res,clientfd);
                                }
                            }
                        }
                        // ***************** DELETE *****************
                        else if(order == "DEL" && msg_vec.size()==3)
                        {
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else
                            {

                            }
                        }
                        else
                            sendData("ERR\n",clientfd);

                        msg_vec.clear();
                    }


                }
            }
        }
    }
    close(clientfd);
}


//******************* COMMUNICATION ************************
void sendData(string msg, int client_fd)
{
    int ret = send(client_fd, msg.c_str(), msg.length(),0);
    if(ret < 0)
    {
        stringstream buf;
        buf << "Sending msg failed: " << strerror(errno);
        throw runtime_error(buf.str().c_str());
    }
}

string recvData(int client_fd)
{
    char buffer[4098];
    int ret;
    string msg="";

    do
    {
        memset(buffer,0,sizeof(char)*4098);
        ret = recv(client_fd,buffer,4097,0);
        if (ret > 0)
        {
            buffer[4097] = 0;
            int i=0;
            while(buffer[i]!=0)
            {
                msg += buffer[i];
                i++;
            }
        }
    }while(ret==4097);
    msg += '\0';

    return msg;
}



// ******************** HELPER FUNCTIONS ***********************
timeval resetSelect(int &clientfd,fd_set &read_fd)
{
    FD_ZERO(&read_fd);
    FD_SET(clientfd,&read_fd);
    timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    return timeout;
}

vector<string> split(string strToSplit, char delimeter)
{
    stringstream ss(strToSplit);
    string item;
    vector<string> splittedStrings;
    while (getline(ss, item, delimeter))
    {
       splittedStrings.push_back(item);
    }
    return splittedStrings;
}
