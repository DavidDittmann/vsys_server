#include "fileController.h"

mutex m;

using namespace std;


//Speichern einer Nachricht im Mailordner des Empfängers
bool safeMsg(string path,string user, string to, string subj, string msg)
{
    string userdir = path+"/"+to;
    cout << "mkdir:" << userdir <<endl;
    //falls nicht vorhanden, erstellen des Mailpoolverzeichnisses und des Userverz.
    mkdir(path.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IRWXO);
    mkdir(userdir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IRWXO);

    //cout << "ok" <<endl;

    ofstream mail;
    //Timestamp in Ticks holen -> Prefix des Filenamens für Sortierung
    unsigned long curTs = (unsigned long)time(NULL);    //current timestamp
    //Erstellen des Filenames aus Timestamp in Ticks + "_" + Hashwert aus
    //Timestamp, Betreff und Thread-ID
    string fileName = userdir+"/"+to_string(curTs)+"_"+createHash(subj)+".txt";

    //Öffnen des Files
    mail.open(fileName.c_str());
    if(mail.is_open())
    {
        //Schreiben der NAchricht in das File
        mail << user <<endl;
        mail << to << endl;
        mail << subj << endl;
        mail << msg << endl;
        mail.close();
        return true;
    }
    //Schliessen des Files
    mail.close();
    return false;
}

//Löschen einer Nachricht
bool delMsg(string path,string user,int msgNumber)
{
    string userdir = path+"/"+user;
    //Erstellen der MAilliste
    vector<string>fileList = getMailsSorted(path,user);
    //Wenn gewünschte Mailnummer zuklein/zugroß -> Rückgabe ERROR
    if(msgNumber>fileList.size()-1 || msgNumber<0)
        throw runtime_error("msgNumber not valid for deleting msg");

    //Wenn Nachricht vorhanden und gelöscht werden konnte -> OK
    string filepath = userdir +"/"+fileList[msgNumber];
    if( remove( filepath.c_str() ) != 0 )
        return false;   //error deleting file
    else
        return true;
}

//Lesen einer bestimmten Nachricht, eines Users
string readMail(string path, string user, int mailNumber)
{
    string userdir = path+"/"+user;
    //Erstellen der Mailliste
    vector<string>fileList = getMailsSorted(path,user);
    //Wenn gewünschte Mailnummer zuklein/zugroß -> Rückgabe ERROR
    if(mailNumber>fileList.size()-1 || mailNumber<0)
        return "ERR\n";

    //Auslesen der NAchricht
    string res = "OK\n";
    auto f = fileList[mailNumber];
    ifstream mail;
    //Öffnen des Files
    mail.open(userdir+"/"+f);
    if(mail.is_open())
    {
        //Lesen bis Ende des Files und in String speichern
        string line;
        while(!mail.eof())
        {
            line ="";
            getline(mail,line);
            res += line +"\n";
        }
    }
    //Schließen des Files
    mail.close();
    return res;
}

//Um die Mails immer in der gleichen Reihenfolge anzeigen zu können,
//haben diese eine Timestamp in Ticks als Prefix vorgestellt und werden
//durch diese Funktion immer nach Namen gleich sortiert zurück gegeben
vector<string> getMailsSorted(string path,string user)
{
    string userdir = path+"/"+user;
    vector <string> filelist;
    DIR *dp;
    struct dirent *dirp;
    //öffnen des User-Mail-Verzeichnisses
    if((dp = opendir(userdir.c_str()))!=NULL)
    {
        //Speichern des Filenames in dem Vector
        while((dirp=readdir(dp)) != NULL)
        {
            string f(dirp->d_name);
            size_t found = f.find("txt");
            if(found!=string::npos)
                filelist.push_back(f);
        }
    }
    closedir(dp);

    //Sortierung des Vektors
    sort(filelist.begin(),filelist.end());
    return filelist;
}

//Rückgabe der Betreffe der gespeicherten Mails
vector<string> listMails(string path,string user)
{
    string userdir = path+"/"+user;
    vector<string> subjList;
    //Liste an Mails sortiert holen
    vector<string>fileList = getMailsSorted(path,user);
    for(auto f : fileList)
    {
        ifstream mail;
        string subj;

        //Aus jeder MAil die 3. Zeile lesen -> =Betreff
        mail.open(userdir+"/"+f);
        if(mail.is_open())
        {
            for(int i=1;i<=3;i++)
                getline(mail,subj);
        }
        subjList.push_back(subj);
        mail.close();
    }
    //Rückgabe der Betreffliste
    return subjList;
}


//Funktion zum erstellen eines EINDEUTIGEN Hashwertes
//bestehend aus dem Betreff, der Thread-ID und einen Timestamp in Ticks
string static createHash(string i_str)
{
    unsigned long curTs = (unsigned long)time(NULL);

    auto myid = this_thread::get_id();
    stringstream ss;
    ss << myid;
    string ThID_String = ss.str();

    hash<string> hasher;
    size_t hashed = hasher(i_str + to_string(curTs) + ThID_String);
    return to_string(hashed);
}

//Auslesen der Ban-Liste, ob eine Client-IP bereits verboten ist
bool isBanned(string poolPath, string userIP)
{
    bool ret=false;
    m.lock();   //Immer nur ein Thread darf schreiben und lesen in dem File

    //Anlegen des Mailpoolverzeichnisses, falls dieses noch nicht vorhanden ist
    mkdir(poolPath.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IRWXO);
    ifstream banOfStream;
    string fileName = poolPath+"/banned.txt";

    //Öffnen des Files
    banOfStream.open(fileName.c_str(),ios::in);
    if(banOfStream.is_open())
    {
        //Lesen jeder Zeile und prüfen, ob die IP dort eingetragen ist
        string line;
        while(!banOfStream.eof())
        {
            line ="";
            getline(banOfStream,line);
            if(line == userIP)
            {
                ret = true;
                break;
            }
        }
    }

    //schliessen des Files
    banOfStream.close();

    m.unlock();

    return ret;
}

//Schreiben der IP eines Clients, welche gebannt werden soll in
//ein File, in dem alle gebannten User stehen
void banIP(string poolPath, string userIP)
{
    m.lock();   //Immer nur einer darf schreiben und lesen in dem File

    //Falls nicht vorhanden, anlegen des Mailpoolverzeichnisses
    mkdir(poolPath.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IRWXO);
    ofstream banOfStream;
    string fileName = poolPath+"/banned.txt";

    //Öffnen der Liste und
    //schreiben der IP in die ban-Liste (anhängend)
    banOfStream.open(fileName.c_str(),ios::out|ios::app);
    if(banOfStream.is_open())
    {
        banOfStream << userIP << endl;
    }

    //schliessen des Files
    banOfStream.close();

    m.unlock();
}
