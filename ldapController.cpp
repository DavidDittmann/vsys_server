#include "ldapController.h"

using namespace std;

bool loginLDAP(string user,string pw,int clientfd)
{
    string result;
    if(searchUser(user,result,clientfd))
    {
        if(verifyUserPW(result,pw))
        {
            //cout << true;
            return true;
        }
        else
        {
            //cout << false;
            return false;
        }
    }
    else
        return false;
}

bool searchUser(string &username, string &res,int clientfd)
{
    LDAP *ld;
    int rc;

    if ( (ld = ldap_init( HOST, PORTNUMBER )) == NULL ) {
        perror( "ldap_init" );
        return false;
    }

    rc = ldap_simple_bind_s(ld,NULL,NULL);
    if ( rc != LDAP_SUCCESS ) {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
        return false;
    }
    char* attrs[4];
    attrs[0] = (char*) "uid";
    attrs[1] = (char*) "ou";
    attrs[2] = (char*) "dc";
    attrs[3] = (char*) NULL;

    string UIDsearch = "uid=" + username;

    LDAPMessage *result;
    rc=ldap_search_s(ld, BASEDN, LDAP_SCOPE_SUBTREE,UIDsearch.c_str(),attrs,0,&result);
    if(rc == LDAP_SUCCESS)
    {
        string completeDN="";
        LDAPMessage *e;
        BerElement *ber;
        char *a;
        char **vals;
        e = ldap_first_entry( ld, result );
        bool tmp=true;

        if(e==NULL)
            return false;
        else
        {
            res = ldap_get_dn(ld,e);
            cout << endl<<res << endl;
        }

        ldap_msgfree(result);
        ldap_unbind(ld);
        return true;
    }
    else
    {
        sendLdapErr(clientfd);
        ldap_msgfree(result);
        ldap_unbind(ld);
        return false;
    }
}
bool verifyUserPW(string &searchDN, string &PW)
{
    LDAP *ld;
    int rc;

    if ( (ld = ldap_init( HOST, PORTNUMBER )) == NULL ) {
        perror( "ldap_init" );
        return NULL;
    }

    rc = ldap_simple_bind_s( ld, searchDN.c_str(), PW.c_str());
    if( rc != LDAP_SUCCESS )
    {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc) );
        ldap_unbind(ld);
        return false;
    }
    else {
        ldap_unbind(ld);
        return true;
    }
}

void sendLdapErr(int clientfd)
{
    string str = "LDAP ERR\n";
    int ret = send(clientfd, str.c_str(), str.length(),0);
    if(ret < 0)
    {
        stringstream buf;
        buf << "Sending msg failed: " << strerror(errno);
        throw runtime_error(buf.str().c_str());
    }
}
