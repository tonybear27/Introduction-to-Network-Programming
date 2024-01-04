#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <sys/socket.h>
#include <set>
#include <queue>
#include <utility>
using namespace std;

vector<string> forbiddenWords = { "==", "superpie", "hello", "starburst stream", "domain expansion" };
string pinnedMessage;

struct User {
    string name;
    string password;
    bool isLoggedIn = 0;
    int loggedInSocket = -1;
    string status = "offline";
};

struct ChatMessage{
    string username;
    string message;
};

struct ChatRoom {
    int roomNumber;
    string owner;
    deque<ChatMessage> chatHistory;
    set<int> clients;
    string pinnedMessage;
};

unordered_map<string, User> registeredUsers;
unordered_map<int, string> clientLoginStatus;
unordered_map<int, ChatRoom> chatRooms;
unordered_map<int, int> clientCurrentRoom;

string toLower(const std::string& str) {
    string lowerStr;
    lowerStr.reserve(str.size());
    transform(str.begin(), str.end(), back_inserter(lowerStr), ::tolower);
    return lowerStr;
}

string maskForbiddenWords(const string& message) {
    
    string maskedMessage = message;
    string lowerMessage = toLower(message); 

    for (const auto& word : forbiddenWords) {
        string lowerWord = toLower(word); 
        string mask(word.length(), '*');
        size_t pos = 0;

        while ((pos = lowerMessage.find(lowerWord, pos)) != string::npos) {
            maskedMessage.replace(pos, word.length(), mask);
            pos += mask.length();
        }
    }

    return maskedMessage;
}

string command(int clientSocket, const string cmd) {
    istringstream iss(cmd);
    vector<string> tokens { istream_iterator<std::string>{iss}, istream_iterator<std::string>{} };

    if(tokens.empty()) return "";

    if(tokens[0] == "register") {
        if(tokens.size() != 3)
            return "Usage: register <username> <password>\n";

        string username = tokens[1];
        string password = tokens[2];

        if (registeredUsers.find(username) != registeredUsers.end()) { return "Username is already used.\n"; }

        registeredUsers[username] = {username, password};

        return "Register successfully.\n";
    
    }
    else if (tokens[0] == "login") {
        if (tokens.size() != 3) return "Usage: login <username> <password>\n";

        string username = tokens[1];
        string password = tokens[2];

        if (clientLoginStatus.find(clientSocket) != clientLoginStatus.end()) { return "Please logout first.\n"; }

        auto it = registeredUsers.find(username);
        if (it == registeredUsers.end() || it->second.password != password) { return "Login failed.\n"; }

        if (it->second.isLoggedIn) { return "Login failed.\n"; }

        it->second.isLoggedIn = true;
        it->second.loggedInSocket = clientSocket;
        clientLoginStatus[clientSocket] = username;
        
        registeredUsers[username].isLoggedIn = true;
        registeredUsers[username].loggedInSocket = clientSocket;
        registeredUsers[username].status = "online";

        return "Welcome, " + username + ".\n";
    }
    else if (tokens[0] == "logout") {
        if (tokens.size() != 1) return "Usage: logout\n";

        auto it = clientLoginStatus.find(clientSocket);
        if (it == clientLoginStatus.end()) { return "Please login first.\n"; }

        string username = it->second;
        registeredUsers[username].isLoggedIn = false;
        registeredUsers[username].loggedInSocket = -1;
        registeredUsers[username].status = "offline";
        clientLoginStatus.erase(clientSocket);
        clientCurrentRoom.erase(clientSocket);
        return "Bye, " + username + ".\n";
    }
    else if (tokens[0] == "exit") {
        if (tokens.size() != 1) return "Usage: exit\n";

        auto it = clientLoginStatus.find(clientSocket);
        if (it != clientLoginStatus.end()) {
            string username = it->second;
            registeredUsers[username].isLoggedIn = false;
            registeredUsers[username].loggedInSocket = -1;
            registeredUsers[username].status = "offline";
            clientLoginStatus.erase(clientSocket);
            string bye = "Bye, " + username + ".\n";
            send(clientSocket, bye.c_str(), bye.length(), 0);

            clientCurrentRoom.erase(clientSocket);
        }

        return "EXIT";

    }
    else if(tokens[0] == "whoami") {
        if(tokens.size() != 1) return "Usage: whoami\n";

        auto it = clientLoginStatus.find(clientSocket);
        if(it == clientLoginStatus.end()) return "Please login first.\n";

        string username = it->second;

        return username + "\n";


    }
    else if (tokens[0] == "set-status") {
        if (tokens.size() != 2) return "Usage: set-status <status>\n";

        auto it = clientLoginStatus.find(clientSocket);
        if (it == clientLoginStatus.end()) { return "Please login first.\n"; }

        string username = it->second;
        string status = tokens[1];

        if (status != "online" && status != "offline" && status != "busy") { return "set-status failed\n"; }

        registeredUsers[username].status = status;
        return username + " " + status + "\n";
    }
    else if (tokens[0] == "enter-chat-room") {
        
        if (tokens.size() != 2) return "Usage: enter-chat-room <number>\n";

        auto loginIt = clientLoginStatus.find(clientSocket);
        if (loginIt == clientLoginStatus.end()) { return "Please login first.\n"; }

        int roomNumber;
        try {
            roomNumber = std::stoi(tokens[1]);
        } catch (...) {
            return "Number " + tokens[1] + " is not valid.\n";
        }

        if (roomNumber < 1 || roomNumber > 100) { return "Number " + tokens[1] + " is not valid.\n"; }

        string username = loginIt->second;

        if (chatRooms.find(roomNumber) == chatRooms.end()) { chatRooms[roomNumber] = ChatRoom{roomNumber, username, {}, {}, ""}; }

        ChatRoom& room = chatRooms[roomNumber];
        room.clients.insert(clientSocket);
        clientCurrentRoom[clientSocket] = roomNumber;  

        string response = "Welcome to the public chat room.\n";
        response += "Room number: " + to_string(roomNumber) + "\n";
        response += "Owner: " + room.owner + "\n";

        auto startIt = room.chatHistory.size() > 10 ? room.chatHistory.end() - 10 : room.chatHistory.begin();

        for (auto it = startIt; it != room.chatHistory.end(); ++it) {
            response += "[" + it->username + "]: " + it->message + "\n";
        }

        response += pinnedMessage;

        string notification = username + " had enter the chat room.\n";
        for (int client : room.clients) if (client != clientSocket) { send(client, notification.c_str(), notification.size(), 0); }

        return response;
    }
    else if (tokens[0] == "list-chat-room") {

        if (tokens.size() != 1) return "Usage: list-chat-room\n";

        if (clientLoginStatus.find(clientSocket) == clientLoginStatus.end()) { return "Please login first.\n"; }

        vector<pair<int, string> > roomList;
        for (const auto& room : chatRooms) { roomList.emplace_back(room.first, room.second.owner); }

        sort(roomList.begin(), roomList.end());

        string response;
        for (const auto& room : roomList) { response += room.second + " " + std::to_string(room.first) + "\n"; }

        return response;
    }
    else if (tokens[0] == "close-chat-room") {

        if (tokens.size() != 2) return "Usage: close-chat-room <number>\n";

        auto loginIt = clientLoginStatus.find(clientSocket);
        if (loginIt == clientLoginStatus.end()) { return "Please login first.\n"; }

        int roomNumber;
        try {
            roomNumber = stoi(tokens[1]);
        } catch (...) {
            return "Chat room " + tokens[1] + " does not exist.\n";
        }

        auto roomIt = chatRooms.find(roomNumber);
        if (roomIt == chatRooms.end()) { return "Chat room " + tokens[1] + " does not exist.\n"; }

        if (roomIt->second.owner != loginIt->second) { return "Only the owner can close this chat room.\n"; }


        string notification = "Chat room " + to_string(roomNumber) + " was closed.\n% ";
        for (int client : roomIt->second.clients) { 
            send(client, notification.c_str(), notification.size(), 0); 
            clientCurrentRoom.erase(client);
        }

        chatRooms.erase(roomIt);

        return "Chat room " + to_string(roomNumber) + " was closed.\n";
    }
    else if (tokens[0] == "list-user") {

        if (tokens.size() != 1) return "Usage: list-user\n";

        if (clientLoginStatus.find(clientSocket) == clientLoginStatus.end()) { return "Please login first.\n"; }

        vector<string> userList;
        for (const auto& user : registeredUsers) { userList.push_back(user.first + " " + user.second.status); }

        sort(userList.begin(), userList.end());
        string result = "";
        for (const auto& user : userList) { result += user + "\n"; }

        return result;
    }
    

    return "Error: Unknown command\n";
}

string chatRoomCmd(int clientSocket, const string& cmd) {

    istringstream iss(cmd);
    vector<string> tokens { istream_iterator<std::string>{iss}, istream_iterator<std::string>{} };

    auto roomIt = clientCurrentRoom.find(clientSocket);

    if(tokens[0] == "exit-chat-room") {

        if (roomIt != clientCurrentRoom.end()) {
            int roomNumber = roomIt->second;
            auto chatRoomIt = chatRooms.find(roomNumber);
            if (chatRoomIt != chatRooms.end()) {
                string username = clientLoginStatus[clientSocket];
                string notification = username + " had left the chat room.\n";
                for (int client : chatRoomIt->second.clients) {
                    if (client != clientSocket) {  
                        send(client, notification.c_str(), notification.size(), 0);
                    }
                }

                chatRoomIt->second.clients.erase(clientSocket);
                clientCurrentRoom.erase(clientSocket);

                return "";
            }
        }

        return "";
    }
    else if (tokens[0] == "list-user") {

        int roomNumber = roomIt->second;
        ChatRoom& room = chatRooms[roomNumber];
            
        string user;
        vector<string> userList;
        for (int client : room.clients) {
            string username = clientLoginStatus[client];
            string status = registeredUsers[username].status;
            user = username + " " + status;
            userList.push_back(user);
        }

        sort(userList.begin(), userList.end());
        string result;
        for (const auto& user : userList) { result += user + "\n"; }

        return result;
    }
    else if(tokens[0] == "pin") {
        string message = maskForbiddenWords(cmd.substr(4));
        int roomNumber = roomIt->second;
        ChatRoom& room = chatRooms[roomNumber];
        string username = clientLoginStatus[clientSocket];
        room.pinnedMessage = "[" + username + "]: " + message + "\n";
        pinnedMessage = "Pin -> " + room.pinnedMessage;
        for (int client : room.clients) {
            send(client, pinnedMessage.c_str(), pinnedMessage.size(), 0);
        }

        return "";
    }
    else if(tokens[0] == "delete-pin") {
        int roomNumber = roomIt->second;
        ChatRoom& room = chatRooms[roomNumber];
        if (room.pinnedMessage.empty()) {
            return "No pin message in chat room " + std::to_string(roomNumber) + "\n";
        }
        room.pinnedMessage.clear();
        pinnedMessage = "";
        
        return "";
    }

    return "Error: Unknown command\n";
}

string processCommand(int clientSocket, const string& cmd) {

    auto roomIt = clientCurrentRoom.find(clientSocket);
    string res = "";

    if (roomIt != clientCurrentRoom.end()) {
        // Client is in a chat room
        int roomNumber = roomIt->second;

        if (!cmd.starts_with("/")) {

            string username = clientLoginStatus[clientSocket];
            string message = "[" + username + "]: " + cmd;
            message = maskForbiddenWords(message);

            ChatRoom& room = chatRooms[roomNumber];

            if (room.chatHistory.size() >= 10) {
                room.chatHistory.pop_front();
            }
            
            string history = cmd.substr(0, cmd.length() - 1); 
            room.chatHistory.push_back({username, maskForbiddenWords(history)});

            for (int client : room.clients) {
                send(client, message.c_str(), message.size(), 0);
            }

            return "";
        }
        else {
            string cm = cmd.substr(1);
            cm.pop_back();
            cout << cm << endl;
            res = chatRoomCmd(clientSocket, cm);
        }
    }
    else
        res = command(clientSocket, cmd);

    return res;
}