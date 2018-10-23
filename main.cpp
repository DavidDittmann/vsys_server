#include "serverController.h"

using namespace std;

int main(int argc, char* argv[]) {

    if(argc!=3)
        cout<< "Server <port> <path to mails>";
    else
    {
        int port = stoi(argv[1]);
        string path = argv[2];

        serverController* Server = new serverController(port,path);

        try{
            Server->run();
        }
        catch(...)
        {
            Server->~serverController();
        }
    }
    return 0;
}


