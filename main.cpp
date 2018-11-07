#include "serverController.h"

using namespace std;

int main(int argc, char* argv[]) {

    //Zum Start des Servers ist zwigend die Angabe eines Mailpool verzeichnisses und eines Ports notwendig
    //Zurzeit nur eine einfaches Mailpoolverzeichnis möglich (bestehende Ordner oder max 1 Unterordner)
    if(argc!=3)
        cout<< "Usage: ./Server <port> <path to mails>";
    else
    {
        //strtol, da stoi nicht exception save ist
        long port = strtol(argv[1],NULL,10);
        if(port > 65535 || port < 1024)
        {
            cout << "Usage: ./Server <port> <path to mails>" << endl;
            cout << "Port musst be greater than 1024 and smaller than 65535" << endl;
            return 0;
        }
        string path = argv[2];

        //initiallisieren des Servers
        int p = (int)port;
        serverController Server(p,path);

        //start des Servers
        try{
            Server.run();
        }
        catch(...)
        {
            cout << "!!!ERROR!!!" << endl;
        }

    }
    return 0;
}


