#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "command.hpp"

#define errquit(m) { cerr << m; exit(-1); }

string welcome = "*********************************\n** Welcome to the Chat server. **\n*********************************\n";
string prompt = "% ";
char buffer[1024] = {0};

int main(int argc, char* argv[]) {

    if(argc < 2) { cerr << "No port given..\n"; return 1; }

    int port = atoi(argv[1]);
    int serverSocket, clientSocket, maxFd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    fd_set readFds;

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) { errquit("Server Socket Error.."); }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { errquit("Socket Option Error.."); }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) { errquit("Bind Error.."); }
    if(listen(serverSocket, 10) < 0) { errquit("Listen Error.."); }

    FD_ZERO(&readFds);
    FD_SET(serverSocket, &readFds);
    maxFd = serverSocket;

    for(; ;) {
        fd_set tmpFds = readFds;
        int activity = select(maxFd + 1, &tmpFds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) { cerr << "Select Error..\n"; }

        if (FD_ISSET(serverSocket, &tmpFds)) {
            if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) { errquit("Accept Error.."); }
            send(clientSocket, welcome.c_str(), welcome.size(), 0);
            send(clientSocket, prompt.c_str(), prompt.size(), 0);
        }

        FD_SET(clientSocket, &readFds);
        if (clientSocket > maxFd) maxFd = clientSocket;
        
        for (int i = 0; i <= maxFd; i++) {
            if (FD_ISSET(i, &tmpFds)) {
                if (i != serverSocket) {
                    memset(buffer, 0, sizeof(buffer));
                    int valRead = read(i, buffer, 1024);
                    if (!valRead) {
                        getpeername(i, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                        close(i);
                        FD_CLR(i, &readFds);
                        clientCurrentRoom.erase(i);
                    } 
                    else {
                        buffer[valRead] = '\0';
                        string response = processCommand(i, buffer);
                        if (response == "EXIT") {
                            close(i);
                            FD_CLR(i, &readFds);
                            clientCurrentRoom.erase(i); 
                            if (i == maxFd) {
                                maxFd = serverSocket;
                                for (int j = 0; j <= maxFd; j++)
                                    if (FD_ISSET(j, &readFds) && j > maxFd) maxFd = j;
                            }
                        } 
                        else if (!response.empty()) send(i, response.c_str(), response.size(), 0);

                        if (clientCurrentRoom.find(i) == clientCurrentRoom.end()) send(i, prompt.c_str(), prompt.size(), 0);
                    }
                }
            }
        }
    }


    return 0;
}