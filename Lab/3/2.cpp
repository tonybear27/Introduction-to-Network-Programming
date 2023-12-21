#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
using namespace std;

vector<string> maze;
int dx[] = {1, -1, 0, 0};
int dy[] = {0, 0, -1, 1};
const string actions[] = {"S", "W", "A", "D"};

struct Point {
    int x, y;
    string path;
};

void saveMaze(string m) {
    istringstream iss(m);
    string line;
    while(getline(iss, line, '\n')) {
        maze.push_back(line+"\n");
    }
}

vector<string> bfs(vector<string>& maze) {
    int n = maze.size();
    int m = maze[0].size();
    vector<string> result;

    queue<Point> q;
    vector<vector<bool>> visited(n, vector<bool>(m, false));

    Point start, exit;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (maze[i][j] == '*') {
                start.x = i;
                start.y = j;
            }
            if (maze[i][j] == 'E') {
                exit.x = i;
                exit.y = j;
            }
        }
    }

    q.push({start.x, start.y, ""});

    while (!q.empty()) {
        Point current = q.front();
        q.pop();

        int x = current.x;
        int y = current.y;

        if (x == exit.x && y == exit.y) {
            result.push_back(current.path);
            return result;
        }

        for (int i = 0; i < 4; i++) {
            int new_x = x + dx[i];
            int new_y = y + dy[i];

            if (new_x >= 0 && new_x < n && new_y >= 0 && new_y < m && maze[new_x][new_y] != '#' && !visited[new_x][new_y]) {
                visited[new_x][new_y] = true;
                q.push({new_x, new_y, current.path + actions[i]});
            }
        }
    }

    return result;
}

int main() {

    const char* serverAddress = "140.113.213.213";
    const int serverPort = 10302;

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

    char buffer[2000];
    int bytesRead=0;
    string response[2];
    string Maze="";

    for (int i=0; i<2; i++){
        bzero(buffer, sizeof(buffer));
        read(clientSocket, buffer, sizeof(buffer));
        response[i]=buffer;
        cout << response[i];
    }
    
    puts("");

    for (int i=239; i<response[0].length(); i++)
        Maze+=response[0][i];
    
    for (int i=0; i<488+26; i++)
        Maze+=response[1][i];

    saveMaze(Maze);
    
    vector<string> ans = bfs(maze);
    vector<string> action;
    
    for (int i=0; i<ans[0].length(); i++) {
        stringstream ss;
        ss << ans[0][i];
        string s = ss.str();
        action.push_back(s+"\n");
    }
    
    string res;
    char buf[4096];
    for (int i=0; i<action.size(); i++) {
        cout << action[i];
        write(clientSocket, action[i].c_str(), sizeof(action[i].c_str()));
        bzero(buf, sizeof(buf));
        read(clientSocket, buf, sizeof(buf));
        res = buf;
        cout << res << endl;
    }

    bzero(buf, sizeof(buf));
    read(clientSocket, buf, sizeof(buf));
    res = buf;
    cout << res;
    

    close(clientSocket);
    

    return 0;
}