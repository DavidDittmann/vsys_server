#include "fileController.h"


using namespace std;

bool safeMsg(string path,string user, string to, string subj, string msg)
{
    string userdir = path+"/"+to;
    cout << "mkdir:" << userdir <<endl;
    mkdir(path.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IRWXO);
    mkdir(userdir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IRWXO);

    cout << "ok" <<endl;

    ofstream mail;
    unsigned long curTs = (unsigned long)time(NULL);    //current timestamp
    string fileName = userdir+"/"+to_string(curTs)+"_"+createHash(subj)+".txt";

    mail.open(fileName.c_str());
    if(mail.is_open())
    {
        mail << user <<endl;
        mail << to << endl;
        mail << subj << endl;
        mail << msg << endl;
        mail.close();
        return true;
    }
    mail.close();
    return false;
}

bool delMsg(string path,string user,int msgNumber)
{

}

string readMail(string path, string user, int mailNumber)
{
    string userdir = path+"/"+user;
    vector<string>fileList = getMailsSorted(path,user);
    if(mailNumber>fileList.size()-1 || mailNumber<0)
        return "ERR\n";

    string res = "OK\n";
    auto f = fileList[mailNumber];
    ifstream mail;
    mail.open(userdir+"/"+f);
    if(mail.is_open())
    {
        string line;
        while(!mail.eof())
        {
            line ="";
            getline(mail,line);
            res += line +"\n";
        }
    }
    mail.close();
    return res;
}
vector<string> getMailsSorted(string path,string user)
{
    string userdir = path+"/"+user;
    vector <string> filelist;
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(userdir.c_str()))!=NULL)
    {
        while((dirp=readdir(dp)) != NULL)
        {
            string f(dirp->d_name);
            size_t found = f.find("txt");
            if(found!=string::npos)
                filelist.push_back(f);
        }
    }
    closedir(dp);
    sort(filelist.begin(),filelist.end());
    return filelist;
}
vector<string> listMails(string path,string user)
{
    string userdir = path+"/"+user;
    vector<string> subjList;
    vector<string>fileList = getMailsSorted(path,user);
    for(auto f : fileList)
    {
        ifstream mail;
        string subj;

        mail.open(userdir+"/"+f);
        if(mail.is_open())
        {
            for(int i=1;i<=3;i++)
                getline(mail,subj);
        }
        subjList.push_back(subj);
        mail.close();
    }
    return subjList;
}

string createHash(string i_str)
{
    unsigned long curTs = (unsigned long)time(NULL);
    pid_t curPid = getpid();
    hash<string> hasher;
    size_t hashed = hasher(i_str + to_string(curTs) + to_string(curPid));
    return to_string(hashed);
}
