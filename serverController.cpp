#include "serverController.h"
#include "ldapController.h"
#include "fileController.h"


/*
* Initiallisierung des Servers und des TCP-Ports
*/
serverController::serverController(int portArg, string pathArg)
{
    this->port = portArg;
    this->path = pathArg;

    this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket() error");
    }
    this->sa.sin_family = AF_INET;  //IPv4
    this->sa.sin_port = htons(this->port);  //Port auf welchen gelauscht werden soll
    this->sa.sin_addr.s_addr = htons(INADDR_ANY);   //Alle eingehenden IP-Addr annehmen

    int one = 1;    //Dynamische Ports verwenden (SO_REUSEADDR)
    if(setsockopt(this->sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))<0)
        cout << "ADDR_REUSE ERROR" <<endl;
    if (bind(this->sock_fd, (struct sockaddr*)&this->sa, sizeof(this->sa)) < 0) {
        perror("bind() error");
    }

    if (listen(this->sock_fd, SOMAXCONN) < 0) {
        perror("listen() error");
    }
}

serverController::~serverController()
{
    //Falls Socket noch nicht geschlossen -> hier schließen
    if(this->sock_fd>0)
        close(sock_fd);

    /*DEPRECATED CODE -> Für Codeanalyse/besprechung noch enthalten
    cout << "Waiting for threads to finish..." << endl;
    int size = this->vec_thread.size();
    for(int i=0;i<size;i++)
        vec_thread[i].join();
    cout << "Threads finished." << endl;

    //this->vec_future.clear();
    //this->vec_thread.clear();
    */
}


/*
* serverController::run() nimmt neue Verbindungen entgegen und startet für
* jede einen neuen handling Thread um die Requests zu bearbeiten
*/
void serverController::run()
{
    cout << "Running..." << endl;
    int conn_fd;
    while(true)
    {
        //Hier wird auf neue eingehende Verbindungen gewartet
        conn_fd = accept(this->sock_fd,0,0);
        if (conn_fd < 0) {
            printf("ERROR at accept() ...\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            //DEPRECATED CODE -> DEPRECATED CODE -> Für Codeanalyse/besprechung noch enthalten
            //promise<bool> promObj;
            //future<bool> futObj = promObj.get_future();

            string pathD = this->path;
            //thread th(connectionHandler,conn_fd,pathD,&promObj);

            //check ob IP gebannt wurde, wenn ja -> Client mitteilen und Verbindung beenden
            //Wenn nicht gebannt -> neuen handling Thread starten und zu Client zuweisen
            if(!isBanned(pathD,string(inet_ntoa(this->sa.sin_addr))))
            {
                thread th(connectionHandler,conn_fd,pathD,this->sa);
                th.detach();
            }
            else
            {
                sendData("BAN\n",conn_fd);
                close(conn_fd);
            }

            //DEPRECATED CODE -> DEPRECATED CODE -> Für Codeanalyse/besprechung noch enthalten
            //this->vec_future.push_back(move(futObj));
            //this->vec_thread.push_back(move(th));
        }
        //DEPRECATED CODE -> DEPRECATED CODE -> Für Codeanalyse/besprechung noch enthalten
        //checkThReady();
    }
}

//FUNCTION DEPRECATED
/*void serverController::checkThReady()
{
    int vecSize = this->vec_future.size();
    for(int i=0;i<vecSize;i++)
    {
        auto status = this->vec_future[i].wait_for(chrono::milliseconds(100));
        if (status == std::future_status::ready) {
            this->vec_thread[i].join();

            this->vec_thread.erase((vec_thread.begin()+i));
            this->vec_future.erase((vec_future.begin()+i));
            vecSize--;
        }
    }
}*/

//Überprüfung ob LDAP credentials passen
bool loginUser(string user, string pw,int client_fd)
{
    //sendData("OK\n", client_fd);//FOR DEBUGING ONLY
    //return true;

    if(loginLDAP(user,pw,client_fd))
    {
        sendData("OK\n",client_fd);
        return true;
    }
    else
    {
        sendData("ERR\n",client_fd);
        return false;
    }
}

/*
* handling Thread für eingehende Verbindungen. Handelt die Requests die von dem zugewiesenen Client kommen
*/
//void connectionHandler(int socket_desc,string path,promise<bool>* promObj)
void connectionHandler(int socket_desc,string path,struct sockaddr_in sa)
{
    cout << "connected: " << socket_desc << endl;
    int clientfd = socket_desc;
    string User = "ERROR_USER";
    int failedLogins = 0;

    fd_set read_fd;
    string msg = "";
    char delim = '\n';
    vector<string>msg_vec;
    string order = "";

    //Es wir dauf Requests gewartet solange keine "QUIT" ankommt
    while(order!="QUIT")
    {
        //Wenn 3x versucht wurde falsch einzuloggen -> IP-Ban
        //und schließen des Ports
        if(failedLogins>2)
        {
            string userIP;
            userIP = string(inet_ntoa(sa.sin_addr));
            banIP(path,userIP);
            order = "QUIT";
        }
        else{
            //Wenn länger als 5 Minuten kein Request kommt -> schließen des Ports
            timeval timeout = resetSelect(clientfd,read_fd);
            int ret = select(clientfd+1,&read_fd,NULL,NULL,&timeout);
            if(ret > 0)
            {
                if(FD_ISSET(clientfd,&read_fd))
                {
                    //Empfangen der Nachricht und speichern der einzelnen Zeilen
                    //in den string-Vector "msg_vec"
                    msg = recvData(clientfd);
                    msg_vec = split(msg,delim);
                    msg_vec.pop_back();
                    if(!msg_vec.empty())
                    {
                        //cout << "User: " << User << endl;
                        order = msg_vec[0];
                        //cout << "msg: " <<  msg << endl;
                        //cout << "msg_vec_size: " << msg_vec.size()<<endl;
                        //cout << "MSGBACK: " << msg_vec.back() << endl;
                        //cout << "Order: " << order << "|" << msg_vec.size() << endl;

                        // **************** LOGIN *********************
                        // besteht aus "LOGIN", Benutzername und Passwort
                        // Wenn LDAP-Login erfolgreich: setzen des Usernames um die anderen Funktionen
                        // auch nützen zu können
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
                            {   //Mitzählen der fehlgeschlagenen Login versuche
                                failedLogins++;
                                cout << "failed login: " << failedLogins << endl;
                            }

                        }
                        // ******************** READ ***********************
                        // lesen einer gespeicherten Nachricht
                        // Read-Befehl besteht aus "READ", Username und Nachrichtennummer
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
                        //speichern einer neuen Nachricht
                        //besteht aus: "SEND", Sender, Empf., Betreff und der Nachricht
                        //abgeschlossen mit einem "\n."
                        else if(order == "SEND" && msg_vec.size()>=6 && msg_vec.back() == ".")
                        {
                            cout << "User: " << User << endl;
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else
                            {   //herauslesen der Nachrichteteile, und speicherung dieser im Mailpool
                                //des Empfängers
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
                        //Anzeige der empfangenen Nachrichten des eingeloggten Users
                        //besteht aus "LIST" + Nachrichtennummer
                        else if(order == "LIST" && msg_vec.size()==2)
                        {
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else
                            {
                                //Auslesen der Anzahl an gespeicherten Nachrichten und wenn
                                //vorhanden die Betreffe
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
                        //Löschen einer Nachricht
                        //enthält: "DELETE", Username und Nachrichtennummer
                        else if(order == "DEL" && msg_vec.size()==3)
                        {
                            if(User == "ERROR_USER")
                                sendData("ERR\n",clientfd);
                            else if(msg_vec[1]==User)
                            {
                                try{
                                    //Wenn löschen erfolgreich -> Rückgabe "OK\n"
                                    int msgNum = stoi(msg_vec[2]);
                                    if(delMsg(path,User,msgNum))
                                        sendData("OK\n",clientfd);
                                    else
                                        sendData("ERR\n",clientfd);
                                }
                                catch(invalid_argument &err)
                                { cout << "Invalid Arg Err: " << err.what() << endl;}
                                catch(runtime_error &err)
                                { cout << "Runtime Err: " << err.what() << endl;}

                            }
                        }
                        else    //Falls etwas unvorhergesehenes kommt -> Rückgabe "ERR\n"
                            sendData("ERR\n",clientfd);

                        //reset des nachrichten vectors
                        msg_vec.clear();
                    }


                }
            }
            else    //Länger als 5 Minuten kein Request des Clients -> automatisches Ende
                order="QUIT";
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

    try{
        do
        {   //Lesen des TCP Ports, bis keine Daten mehr vorhanden
            memset(buffer,0,sizeof(char)*4098);
            ret = recv(client_fd,buffer,4097,0);
            if (ret > 0)
            {
                buffer[4097] = 0;   //Bufferoverflow safety
                int i=0;
                while(buffer[i]!=0)
                {
                    msg += buffer[i];   //Zusammenbau der Nachricht in einen String, für späteres parsing
                    i++;
                }
            }
            else if(ret == 0)   //Socket closed
            {
                stringstream buf;
                buf << "Could not recieve msg - socket closed";
                throw runtime_error(buf.str().c_str());
            }
            else if(ret < 0)    //Error at recv
            {
                stringstream buf;
                buf << "Recieving msg failed: " << strerror(errno);
                throw runtime_error(buf.str().c_str());
            }
        }while(ret==4097);
        msg += '\0';
    }catch(runtime_error &err)
    { msg = ""; cout << err.what() << endl;}

    return msg;
}



// ******************** HELPER FUNCTIONS ***********************
timeval resetSelect(int &clientfd,fd_set &read_fd)
{   //Setzens der Wartezeit beim Select für eingehende Nachrichten auf 300 Sekunden = 5 Minuten
    FD_ZERO(&read_fd);
    FD_SET(clientfd,&read_fd);
    timeval timeout;
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    return timeout;
}

//split teilt einen String durch einen bestimmten delimiter und gibt diesen als vector zurück
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
