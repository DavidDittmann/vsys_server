#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LDAP_DEPRECATED 1
#include <ldap.h>

#define HOST "ldap.technikum-wien.at"
#define PORTNUMBER 389
#define BASEDN "dc=technikum-wien,dc=at"

bool loginLDAP(std::string user,std::string pw,int clientfd);
bool searchUser(std::string &username, std::string &res,int clientfd);
bool verifyUserPW(std::string &searchDN,std::string &PW);
void sendLdapErr(int clientfd);
