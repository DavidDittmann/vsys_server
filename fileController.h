#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <vector>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <mutex>
#include <thread>

using namespace std;

bool safeMsg(string path,string user,string to,string subj, string msg);
bool delMsg(string path,string user, int msgNumber);
string readMail(string path, string user, int mailNumber);
vector<string> getMailsSorted(string path,string user);
vector <string> listMails(string path,string user);
static string createHash(string i_str);
bool isBanned(string poolPath, string userIP);
void banIP(string poolPath, string userIP);
