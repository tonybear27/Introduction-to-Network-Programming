#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

char buffer[2000];
//const char* serverAddress = "140.113.213.213";
//const int serverPort = 10314;

const char* serverAddress = "172.21.0.4";
const int serverPort = 10001;

char* findOTP() {

    const char* delimiter = "11";
    char* delimiterPos = strstr(buffer, delimiter);

    if (delimiterPos != NULL)
        delimiterPos += strlen(delimiter);

	delimiterPos += 5;
	return delimiterPos;
}

void getOTP() {
    const char* path = "/otp?name=Chloe";

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverAddress, &(serverAddr.sin_addr)) == -1) {
        perror("Invalid address");
        close(clientSocket);
        return;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to the server");
        close(clientSocket);
        return;
    }

    char request[4096];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n\r\n",
        path, serverAddress
    );

    send(clientSocket, request, strlen(request), 0);

    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        continue;

    close(clientSocket);
}

void saveFile(const char* message) {

    const char* filename = "otp.txt";

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening the file");
        return;
    }

    fprintf(file, "%s", message);

    fclose(file);
}

void sendOTP(const char* mes) {

    const char* path = "/upload";

    // Create Socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        return;
    }

    // Connect to the server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverAddress, &(serverAddr.sin_addr)) == -1) {
        perror("Invalid address");
        close(clientSocket);
        return;
    }
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to the server");
        close(clientSocket);
        return;
    }

    // Send the HTTP header
    char body[2000];
    snprintf(body, sizeof(body), 
        "--myboundary\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"otp.txt\"\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "%s\r\n"
        "--myboundary--\r\n",
        mes);

    char header[2048];
    snprintf(header, sizeof(header), 
        "POST /upload HTTP/1.1 \r\n"
        "Host: %s:%d\r\n"
        "Content-Type: multipart/form-data; boundary=myboundary\r\n"
        "Content-Length: %ld\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n"
        "%s", 
        serverAddress, serverPort, strlen(body), body);

    send(clientSocket, header, strlen(header), 0);


    // Get the server's response
    char response[1024];
    ssize_t bytes_received = recv(clientSocket, response, sizeof(response), 0);
    if (bytes_received < 0)
        perror("Error receiving data from the server");
    else {
        response[bytes_received] = '\0';
        printf("\n%s", response);
    }

    close(clientSocket);

}

int main() {
    
	getOTP();
	char *password = findOTP();
    printf("%s", password); 
    //saveFile(password);
    sendOTP(password);
    return 0;
}