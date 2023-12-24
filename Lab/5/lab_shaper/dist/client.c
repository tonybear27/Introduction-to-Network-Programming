#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define PORT 123
#define MAX 200000

int compareDoubles(const void *a, const void *b) {
    double difference = (*(double *)a - *(double *)b);
    if (difference > 0) {
        return 1;
    } else if (difference < 0) {
        return -1;
    } else {
        return 0;
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    struct timeval startLat, endLat, start, end;

    //long long chunkSize = 1;
    double bandList[10];
    double bestLatency = 1e6;
    double bandwidth = 0;
    char mes[MAX];
    char ack[2];
    char bd[MAX];
    char lat[2] = "A";
    memset(mes, 'A', MAX);
    bzero(ack, 2);

    /**** Measure Latency ****/
    for (int i = 0; i < 10; i++) 
    {
        bzero(ack, 2);
        gettimeofday(&startLat, 0);
        send(clientSocket, lat, 2, 0);
        recv(clientSocket, ack, 2, 0);
        gettimeofday(&endLat, 0);

        double latency = 1000 * ((endLat.tv_sec - startLat.tv_sec) + (endLat.tv_usec - startLat.tv_usec) / 1e6 );
        if (latency < bestLatency)
            bestLatency = latency;
    }

    /**** Bandwidth ****/
    for (int i = 0; i < 10; i++) {
        gettimeofday(&start, 0);
        int rec = recv(clientSocket, bd, MAX, 0);
        gettimeofday(&end, 0);
        double elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        bandwidth = (8 * rec) / elapsedTime / 1e6;
        bandList[i] = bandwidth;
    }
    
    qsort(bandList, sizeof(bandList) / sizeof(bandList[0]), sizeof(bandList[0]), compareDoubles);
    //for(int i = 0; i< 10; i++)
    //    printf("%d: %lf\n", i, bandList[i]);
    printf("# RESULTS: delay = %lf ms, bandwidth = %lf Mbps\n", bestLatency / 2, bandList[6]); 
    
    close(clientSocket);
    
    sleep(3);

    return 0;
}
