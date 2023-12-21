#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <sys/time.h>

using namespace std;


string res = "";

bool dfs(int nowy, int nowx, vector<string> &maze , string &answer) {
    if(maze[nowy][nowx] == 'E') return true;
    maze[nowy][nowx] = '+';
    if(maze[nowy+1][nowx] == '.' || maze[nowy+1][nowx] == 'E') {
        if(dfs(nowy+1, nowx, maze, answer)) {
            answer.append("S");
            return true;
        }
    }
    if(maze[nowy-1][nowx] == '.' || maze[nowy-1][nowx] == 'E') {
        if(dfs(nowy-1, nowx, maze, answer)) {
            answer.append("W");
            return true;
        }
    }
    if(maze[nowy][nowx+1] == '.' || maze[nowy][nowx+1] == 'E') {
        if(dfs(nowy, nowx+1, maze, answer))
        {
            answer.append("D");
            return true;
        }
    }
    if(maze[nowy][nowx-1] == '.' || maze[nowy][nowx-1] == 'E') {
        if(dfs(nowy, nowx-1, maze, answer))
        {
            answer.append("A");
            return true;
        }
    }

    maze[nowy][nowx] = '.';
    return false;
}

bool dfs3(int nowy, int nowx, vector<string> &maze , string &answer, int clientSocket) {
    if(maze[nowy][nowx] == 'E') 
        return true;
    maze[nowy][nowx] = '+';

    if(maze[nowy+1][nowx] == '.' || maze[nowy+1][nowx] == 'E') {
        string tt = "S\n";
        string r = "";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        char buffer[150];
        int bytesRead;
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        string::size_type find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
        stringstream sssss;
        sssss << r;
        vector<string> v;
        string ts;
        while(sssss >> ts)
            if(ts[0] == '.' || ts[0] == '*' || ts[0] == '#' || ts[0] == 'E'&& ts[1] !='n' )  
                v.push_back(ts);  
        int cenx = 0;
        int ceny = 0;
        for(int i=0; i<v.size(); i++) {
            for(int j=0; j<v[i].size(); j++) {
                if(v[i][j] == '*') {
                    ceny = i;
                    cenx = j;
                }
            }
        }
        
        for(int i=0; i<v.size(); i++)
            for(int j=0; j<v[i].size(); j++)
                if(maze[nowy-ceny+1+i][nowx-cenx+j] != '+' && v[i][j] != '*') 
                    maze[nowy-ceny+1+i][nowx-cenx+j] = v[i][j];

        if(dfs3(nowy+1, nowx, maze, answer, clientSocket)) {
            answer.append("S");
            return true;
        }

        tt = "W\n";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        r = "";
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
    }
    if(maze[nowy-1][nowx] == '.' || maze[nowy-1][nowx] == 'E') {
        string tt = "W\n";
        string r = "";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        char buffer[150];
        int bytesRead;
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        string::size_type find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
        stringstream sssss;
        sssss << r;
        vector<string> v;
        string ts;
        while(sssss >> ts)
            if(ts[0] == '.' || ts[0] == '*' || ts[0] == '#' || ts[0] == 'E'&&ts[1] !='n' )  
                v.push_back(ts);  
        int cenx = 0;
        int ceny = 0;
        for(int i=0; i<v.size(); i++){
            for(int j=0; j<v[i].size(); j++){
                if(v[i][j] == '*'){
                    ceny = i;
                    cenx = j;
                }
            }
        }

        for(int i=0; i<v.size(); i++){
            for(int j=0; j<v[i].size(); j++){
                if(maze[nowy-ceny-1+i][nowx-cenx+j] != '+' && v[i][j] != '*') 
                    maze[nowy-ceny-1+i][nowx-cenx+j] = v[i][j];
            }
        }

        if(dfs3(nowy-1, nowx, maze, answer,clientSocket)){
            answer.append("W");
            return true;
        }
        tt = "S\n";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        r = "";
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
    }

    if(maze[nowy][nowx+1] == '.' || maze[nowy][nowx+1] == 'E') {
        string tt = "D\n";
        string r = "";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        char buffer[150];
        int bytesRead;
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        string::size_type find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
        stringstream sssss;
        sssss << r;
        vector<string> v;
        string ts;
        while(sssss >> ts)
            if(ts[0] == '.' || ts[0] == '*' || ts[0] == '#' || ts[0] == 'E'&&ts[1] !='n' )  v.push_back(ts);  
        int cenx = 0;
        int ceny = 0;
        for(int i=0; i<v.size(); i++){
            for(int j=0; j<v[i].size(); j++){
                if(v[i][j] == '*'){
                    ceny = i;
                    cenx = j;
                }
            }
        }

        for(int i=0; i<v.size(); i++){
            for(int j=0; j<v[i].size(); j++){
                if(maze[nowy-ceny+i][nowx-cenx+1+j] != '+' && v[i][j] != '*') 
                    maze[nowy-ceny+i][nowx-cenx+1+j] = v[i][j];
            }
        }
        
        if(dfs3(nowy, nowx+1, maze, answer, clientSocket)) {
            answer.append("D");
            return true;
        }
        tt = "A\n";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        r = "";
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
    }

    if(maze[nowy][nowx-1] == '.' || maze[nowy][nowx-1] == 'E') {
        string tt = "A\n";
        string r = "";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        char buffer[150];
        int bytesRead;
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        string::size_type find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r;
        stringstream sssss;
        sssss << r;
        vector<string> v;
        string ts;
        while(sssss >> ts) { 
            if(ts[0] == '.' || ts[0] == '*' || ts[0] == '#' || ts[0] == 'E'&&ts[1] !='n' ) {
                v.push_back(ts);  
            }
        }
        
        int cenx = 0;
        int ceny = 0;
        for(int i=0; i<v.size(); i++) {
            for(int j=0; j<v[i].size(); j++) {
                if(v[i][j] == '*') {
                    ceny = i;
                    cenx = j;
                }
            }
        }

        for(int i=0; i<v.size(); i++) {
            for(int j=0; j<v[i].size(); j++) {
                if(maze[nowy-ceny+i][nowx-cenx-1+j] != '+' && v[i][j] != '*') 
                    maze[nowy-ceny+i][nowx-cenx-1+j] = v[i][j];

            }
        }
        if(dfs3(nowy, nowx-1, maze, answer, clientSocket)) {
            answer.append("A");
            return true;
        }
        tt = "D\n";
        send(clientSocket, tt.c_str(), tt.size(), 0);
        r = "";
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        find = r.find("BINGO");
        if (find != std::string::npos) 
            res += r; 
    }
    
    maze[nowy][nowx] = '.';
    return false;
}

int main() 
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) 
    {
        cerr << "Socket creation failed." << endl;
        return 1;
    }

    struct hostent *server = gethostbyname("inp.zoolab.org");
    if (server == nullptr) 
    {
        cerr << "Failed to resolve server address." << endl;
        return 1;
    }

    struct sockaddr_in serverAddress;
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(10304);
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

    //connect
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0) 
    {
        cerr << "Connection failed." << endl;
        return 1;
    }

    // receive from server
    char buffer[5000];
    int bytesRead;
    string r = "";

    bool f = false;
    while( f == false && (bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        r.append(buffer, bytesRead);
        for(int i=0; i<5000; i++) {
            if(buffer[i] == '>' && buffer[i-1] == ')') {
                f = true;
                break;
            }

        }
    }

    string answer_temp;
    int x, y, viewx, viewy;
    stringstream ss;
    ss.clear();
    ss.str("");
    ss << r;
    string s;
    int c=0;
    vector<string> receive;
    int casee = 0;
    while(ss >> s) {
        if(c==1) 
            casee = s[1]-'0'; 
        receive.push_back(s);
        c++;
    }
    vector<string> maze;
    int startx = 0, starty = 0;
    int endx = 0, endy = 0;

    if(casee == 1 || casee == 2) {
        for(int i=0; i<receive.size(); i++) {
            if(receive[i] == "maze" && receive[i+1] == "=") {
                ss.clear();
                ss.str("");
                ss << receive[i+2];
                ss >> x;
                ss.clear();
                ss.str("");
                ss << receive[i+4];
                ss >> y;
                maze.resize(y);
            }
            else if(receive[i].substr(0,2) == "##")
            {
                for(int j=0; j<y; j++)
                    maze[j] = receive[i++];
                i--;
            }
        }
    }
    else if(casee == 3)
    {
        for(int i=0; i<receive.size(); i++)
        {
            if(receive[i] == "maze" && receive[i+1] == "=") 
            {
                ss.clear();
                ss.str("");
                ss << receive[i+2];
                ss >> x;
                ss.clear();
                ss.str("");
                ss << receive[i+4];
                ss >> y;
                maze.resize(y);
                for(int tt=0; tt<y; tt++)  
                    maze[tt].resize(x*2+5);
            }
            if(receive[i] == "area" && receive[i+1] == "=")
            {
                ss.clear();
                ss.str("");
                ss << receive[i+2];
                ss >> viewx;
                ss.clear();
                ss.str("");
                ss << receive[i+4];
                ss >> viewy;
                maze.resize(y);
                for(int j=0; j<y; j++)  maze[j].resize(x);
                int t;
                ss.clear();
                ss.str("");
                ss << receive[i+5].substr(0, receive[i+5].size()-1);    //avoid :
                ss >> t;
                string toreset = "";
                if(t>0) 
                    for(int _=0; _<t; _++)  
                        toreset += 'I';
                else    
                    for(int _=t; _<0; _++)  
                        toreset += 'K';
                for(int _=0; _<x-viewx; _++)    
                    toreset += 'J';
                toreset += '\n';
                send(clientSocket, toreset.c_str(), toreset.size(), 0);
                break;
            }
        }
        

        string goright = "";
        for(int _=0; _<viewx; _++)   
            goright += 'L';
        string godown  = "";
        for(int _=0; _<viewy; _++)   
            godown += 'K';
        string goleft = "";
        for(int _=0; _<x+viewx+1; _++)   
            godown += 'J';
        
        char buffer[5000];
        int startmaze = 0;
        int filly = 0;
        while(filly <= y && startmaze <=x) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            stringstream sssss;
            sssss << buffer;
            vector<string> v;
            string ts;
            while(sssss >> ts) { 
                if(ts[0] == '.' || ts[0] == '*' || ts[0] == '#' || ts[0] == 'E'&&ts[1] !='n' )  
                    v.push_back(ts);    
            }
            
            if(v.empty())
            { 
                send(clientSocket, (goright+'\n').c_str(),(goright+'\n').size(), 0);
                continue;
            }
            else 
            {
                for(int p=0; p<v.size(); p++)
                {
                    for(int _=0; _<v[p].size(); _++)
                    {
                        maze[filly][startmaze] = v[p][_];
                        if(v[p][_] == 'E') {
                            endx = startmaze; 
                            endy = filly;
                        }
                        if(v[p][_] == '*') {
                            startx = startmaze; 
                            starty = filly;
                        }
                        startmaze++;
                    }
                    filly+=1;
                    startmaze -= v[p].size();
                }
                filly -= v.size();
                startmaze += v[0].size();
                if(startmaze >= x) {
                    startmaze=0;
                    filly += viewy;
                    send(clientSocket, (godown+goleft+'\n').c_str(), (godown+goleft+'\n').size(), 0);
                }
                else 
                    send(clientSocket, (goright+'\n').c_str(), (goright+'\n').size(), 0);
            }
        }
        r = "";
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        r.append(buffer, bytesRead);
    }
    else if(casee == 4) { 
        x = 210;
        y = 210;
        maze.resize(y);
        for(int i=0; i<y; i++)  
            maze[i].assign(x,'X');
        int centerx = 100;  
        int centery = 100;  
        for(int i=0; i<receive.size(); i++) {
            if(receive[i] == "7" && receive[i-1] == "x" && receive[i-2] == "11")  {
                for(int _=0; _<7; _++) {
                    string temp;
                    ss.clear();
                    ss.str("");
                    ss << receive[i+2];
                    ss >> temp;
                    for(int j=0; j<11; j++)
                        maze[centery+_][centerx+j] = temp[j]; 
                    i+=2;
                }
            }
        }
        
        cout << dfs3(103, 105, maze, answer_temp, clientSocket) << "!!\n";;
        maze[103][105] = '*';
        
    }

    if(casee ==1 || casee == 2) {
         for(int i=0; i<y; i++) {
            for(int j=0; j<x; j++) {
                if(maze[i][j] == '*') {
                    startx = j;
                    starty = i;
                }
                if(maze[i][j] == 'E') {
                    endx = j;
                    endy = i;
                }
            }
        }
    }

    if(casee != 4) {
        dfs(starty, startx, maze, answer_temp);
        maze[starty][startx] = '*';
    }

    string answer = "";
    for(int i=0; i<answer_temp.size(); i++)
        answer += answer_temp[answer_temp.size()-1-i];
    answer += "\n";
    cout << "answer: " << answer;
    
    int bytesRead2;
    send(clientSocket, answer.c_str(), answer.size(), 0);

    int timeout = 1000; 

    clock_t startTime = clock();

    while (true) {
        char buffer2[10000];
        bytesRead2 = recv(clientSocket, buffer2, sizeof(buffer2), 0);
        if (bytesRead2 > 0) 
            res.append(buffer2, bytesRead2);

        clock_t currentTime = clock();
        if ((currentTime - startTime) / CLOCKS_PER_SEC * 1000 >= timeout) 
            break;
    }


    cout << res << endl;

    close(clientSocket);

    return 0;
}

