#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <regex>
#include <stack>
using namespace std;

string maze;
int rows=7, column=11;

struct Point {
    int x, y;
};

int main() {

    const char* serverAddress = "140.113.213.213";
    const int serverPort = 10301;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating socket" << endl;
        return 1;
    }

    sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverAddress, &serverInfo.sin_addr) <= 0) {
        cerr << "Invalid server address" << endl;
        return 1;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == -1) {
        cerr << "Error connecting to the server" << endl;
        return 1;
    }

    char buffer[4096];
    int bytesRead=0;
    string response;

    for (int i=0; i<2; i++){
        read(clientSocket, buffer, sizeof(buffer));
        response=buffer;
        cout << response;
    }
    Point START, END;
    
    regex mazeRegex("#{11}\\n(.+\\n)+#{11}");
    smatch match;
    vector<vector<char> > mazeVector;
    int start=0, end=0;
    if (regex_search(response, match, mazeRegex))
        maze = match[0].str();
    while ((end = maze.find('\n', start)) != string::npos) {
            string row = maze.substr(start, end - start);
            vector<char> mazeRow(row.begin(), row.end());
            mazeVector.push_back(mazeRow);
            start = end + 1;
        }

    for (int i = 0; i < mazeVector.size(); i++) {
            for (int j = 0; j < mazeVector[i].size(); j++) {
                if (mazeVector[i][j] == '*') {
                    START.x=i;
                    START.y=j;
                } else if (mazeVector[i][j] == 'E') {
                    END.x=i;
                    END.y=j;
                }
            }
        }

    vector<string> action;

    int changeX=END.x-START.x;
    int changeY=END.y-START.y;

    while(changeX!=0) {
        if (changeX<0) {
            changeX+=1;
            action.push_back("W\n");
        }
        else {
            changeX-=1;
            action.push_back("S\n");
        }
    }
    while(changeY!=0) {
        if (changeY<0) {
            changeY+=1;
            action.push_back("A\n");
        }
        else {
            changeY-=1;
            action.push_back("D\n");
        }
    }

    for (int i=0; i<action.size(); i++) {
       
        write(clientSocket, action[i].c_str(), sizeof(action[i].c_str()));
        bzero(buffer, sizeof(buffer));
        read(clientSocket, buffer, sizeof(buffer));
        response = buffer;
        cout << response;
    }

    bzero(buffer, sizeof(buffer));
    read(clientSocket, buffer, sizeof(buffer));
    response = buffer;
    cout << response;


    close(clientSocket);

    return 0;
}
