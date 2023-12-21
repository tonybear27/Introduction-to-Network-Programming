#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <limits.h>

#define PORT 123
#define MAX 200000

int main() {

        int serverSocket, clientSocket;
        struct sockaddr_in serverAddr, clientAddr;
        socklen_t addr_size = sizeof(struct sockaddr_in);

        if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Error binding socket");
            exit(EXIT_FAILURE);
        }

        if (listen(serverSocket, 10) == -1) {
            perror("Error listening for connections");
            exit(EXIT_FAILURE);
        }

        while (1) {

            if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size)) == -1) {
                perror("Error accepting connection");
                exit(EXIT_FAILURE);
            }

            char mes[MAX];
            memset(mes, 'A', MAX);

            char lat[2];
            bzero(lat, 2);

            for (int i = 0; i < 10; i++) 
            {
                recv(clientSocket, lat, 2, 0);
                send(clientSocket, "O", 2, 0);
            }

            for (int i = 0; i < 15; i++) {
                send(clientSocket, mes, MAX, 0);
                //sleep(1);
            }

        }

        close(serverSocket);

    return 0;
}
