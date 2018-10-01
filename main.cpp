#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include <iostream>

using namespace std;

int main() {
    signal(SIGPIPE, SIG_IGN);
    int ln = socket(AF_INET, SOCK_STREAM, 0);
    if (ln < 0) {
        perror("socket() error");
        return -1;
    }
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(8080);
    sa.sin_addr.s_addr = htons(INADDR_ANY);

    int one = 1;
    setsockopt(ln, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(ln, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("bind() error");
        return -1;
    }

    if (listen(ln, SOMAXCONN) < 0) {
        perror("listen() error");
        return -1;
    }

    cout << "Socked num: " << ln << endl;
    char buf[100];

    while (true) {
        cout << "Will accept" << endl;
        int conn = accept(ln, 0, 0);
        if (conn < 0) {
            perror("accept() error");
        }

        int fail = false;
        while (!fail) {
            int n = recv(conn, &buf, sizeof(buf) - 1, 0);
            if (n < 0) {
                perror("recv() error");
                break;
            }
            if (n == 0) {
                break;
            }
            buf[n] = '\0';
            cout << "New data from client: " << buf << endl;
            int total = n;
            int written = 0;
            while (written < total + 1) {
                int n = send(conn, buf + written, total + 1 - written, 0);
                if (n < 0) {
                    perror("send() error");
                    fail = true;
                    break;
                }
                if (n == 0) {
                    cout << "Eof received" << endl;
                }
                written += n;
            }
        }
        cout << "Client disconnected" << endl;
    }

    return 0;
}