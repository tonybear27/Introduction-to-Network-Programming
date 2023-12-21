#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#define PACKETSIZE 400
#define MAXLINE 35000
#define SMG 80
#define err_quit(m) { perror(m); exit(-1); }

using namespace std;

struct perFile{
    char header[8];
    char packet[PACKETSIZE];
    int len = 0;
};