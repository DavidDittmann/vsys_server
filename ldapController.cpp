#include "ldapController.h"

//TODO: Check ob alle Fehlerfälle behandelt

using namespace std;


//LDAP-Login erfolgt in 3 Schritten:
// 1) Anonyme Suche des Users im LDAP-Verzeichnis
// 2) Wenn gefunden -> Verzeichnisstruktur holen
// 3) Checken ob Passwort korrekt ist
bool loginLDAP(string user,string pw,int clientfd)
{
    string result;
    //User suchen
    if(searchUser(user,result,clientfd))
    {
        //User wurde gefunden -> Bestätigung des Passworts
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
    else    //Wenn nicht gefunden -> login fehlgeschlagen
        return false;
}


//Suchen des Users im LDAP-Verz. und wenn vorhanden, Verzeichnisstruktur holen
bool searchUser(string &username, string &res,int clientfd)
{
    LDAP *ld;
    int rc;

    //Verbindung mit LDAP-Server aufbauen
    if ( (ld = ldap_init( HOST, PORTNUMBER )) == NULL ) {
        perror( "ldap_init" );
        return false;
    }

    //Anonyme Suche durchführen
    //ACHTUNG: NUR IM INTRANET MÖGLICH
    rc = ldap_simple_bind_s(ld,NULL,NULL);
    if ( rc != LDAP_SUCCESS ) {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
        return false;
    }

    //Festlegen der Attribute die gesucht werden
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
        //Wenn suche erfolgreich -> Verzeichnis holen mittels "ldap_first_entry(...)"
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
            //speichern des Verzeichnisses, für folgende Passwortabfrage
            res = ldap_get_dn(ld,e);
            cout << endl<<res << endl;
        }

        //Immer verbindung zum LDAP-Server abbauen
        ldap_msgfree(result);
        ldap_unbind(ld);
        return true;
    }
    else
    {
        //Immer verbindung zum LDAP-Server abbauen
        (clientfd);
        ldap_msgfree(result);
        ldap_unbind(ld);
        return false;
    }
}


//Verifiezierung des Passworts zum User
bool verifyUserPW(string &searchDN, string &PW)
{
    LDAP *ld;
    int rc;

    //Neue Verbindung zum LDAP-Server aufbauen
    if ( (ld = ldap_init( HOST, PORTNUMBER )) == NULL ) {
        perror( "ldap_init" );
        return NULL;
    }

    //Verbindung mit den Usercredentials aufbauen, wenn positiv -> PW korrekt
    //Auf jeden Fall die Verbindung wieder trennen
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

