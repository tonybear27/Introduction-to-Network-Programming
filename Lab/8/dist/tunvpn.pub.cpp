/*
 *  Lab problem set for INP course
 *  by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 *  License: GPLv2
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <sys/types.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <map>
using namespace std;

#define NIPQUAD(m)	((unsigned char*) &(m))[0], ((unsigned char*) &(m))[1], ((unsigned char*) &(m))[2], ((unsigned char*) &(m))[3]
#define errquit(m)	{ perror(m); exit(-1); }

#define MYADDR		0x0a0000fe
#define ADDRBASE	0x0a00000a
#define	NETMASK		0xffffff00

unsigned int lastAssigned = ADDRBASE;

struct thread_args {
    int tunFd;
    int clientSocket;
};

map<unsigned int, int> routingTable;

void debug(const struct ip *ip_header) {

    char src_ip[INET_ADDRSTRLEN]; 
    char dst_ip[INET_ADDRSTRLEN]; 

    inet_ntop(AF_INET, &(ip_header->ip_src), src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_header->ip_dst), dst_ip, INET_ADDRSTRLEN);

    printf("Source IP: %s\n", src_ip);
    printf("Destination IP: %s\n\n", dst_ip);
}

void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(EXIT_FAILURE);
    }
}

int tun_alloc(char *dev) {
	struct ifreq ifr;
	int fd, err;
	if((fd = open("/dev/net/tun", O_RDWR)) < 0 )
		return -1;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI | IFF_UP;	/* IFF_TUN (L3), IFF_TAP (L2), IFF_NO_PI (w/ header) */
	if(dev && dev[0] != '\0') strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
		close(fd);
		return err;
	}
	if(dev) {
		ifr.ifr_name[IFNAMSIZ - 1] = '\0';
		strcpy(dev, ifr.ifr_name);
	}
	return fd;
}

int ifreq_set_mtu(int fd, const char *dev, int mtu) {
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_mtu = mtu;
	if(dev) {
		ifr.ifr_name[IFNAMSIZ - 1] = '\0';
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}
	return ioctl(fd, SIOCSIFMTU, &ifr);
}

int ifreq_get_flag(int fd, const char *dev, short *flag) {
	int err;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	if(dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if(err == 0) {
		*flag = ifr.ifr_flags;
	}
	return err;
}

int ifreq_set_flag(int fd, const char *dev, short flag) {

	short current_flags;
    ifreq_get_flag(fd, dev, &current_flags);
    current_flags |= flag;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    if (dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    ifr.ifr_flags = current_flags;
    return ioctl(fd, SIOCSIFFLAGS, &ifr);
}

int ifreq_set_sockaddr(int fd, const char *dev, int cmd, unsigned int addr) {
	struct ifreq ifr;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	memset(&ifr, 0, sizeof(ifr));
	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
	if(dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);

	return ioctl(fd, cmd, &ifr);
}

int ifreq_set_addr(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFADDR, addr);
}

int ifreq_set_netmask(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFNETMASK, addr);
}

int ifreq_set_broadcast(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFBRDADDR, addr);
}

void *handleClient(void *args) {
	
	int *fds = (int *)args;
    int tunFd = fds[0];
    int clientSocket = fds[1];

	setNonBlocking(clientSocket);
   	setNonBlocking(tunFd);
	while(1) {
		char buffer[20480];
		int nread = read(clientSocket, buffer, sizeof(buffer));
		
		if(nread > 0) {
			// struct ip *iph = (struct ip *)buffer;
			// printf("Server Receive\n");
			// debug(iph);
			write(tunFd, buffer, nread);
		}

		char buf[20480];
		int nnread = read(tunFd, buf, sizeof(buf));
		if(nnread > 0) {
			struct ip *ip_hdr = (struct ip *)buf;
			unsigned int dest_ip = ip_hdr->ip_dst.s_addr;
			// printf("Server Send\n");
			// debug(ip_hdr);
			int tempSocket = routingTable[dest_ip];
			write(tempSocket, buf, nnread);
		}
	}
}

int tunvpn_server(int port) {

	// XXX: implement your server codes here ...

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) errquit("Server Socket Error..");

	fprintf(stderr, "## [server] starts ...\n");

	struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any interface
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) { errquit("Bind Error.."); }

    if (listen(serverSocket, SOMAXCONN) < 0) { errquit("Listen Error.."); }

	char tunName[] = "tun0";
	int tunFd = tun_alloc(tunName);
	if(tunFd < 0) errquit("tun0 Create Error..");
	
	ifreq_set_mtu(serverSocket, tunName, 1400);

	ifreq_set_addr(serverSocket, tunName, htonl(MYADDR));
	ifreq_set_netmask(serverSocket, tunName, htonl(NETMASK));
	ifreq_set_broadcast(serverSocket, tunName, htonl(0x0a0000ff));
	ifreq_set_flag(serverSocket, tunName, IFF_UP);

	routingTable[htonl(MYADDR)] = serverSocket;

    while(1) { 

		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
		if (clientSocket < 0) {
			perror("Accept Error..");
			continue;
		}

		unsigned int clientIP = htonl(lastAssigned++);
		send(clientSocket, &clientIP, sizeof(clientIP), 0);

		struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));
		if (!args) {
            perror("Failed to allocate memory for thread arguments");
            close(clientSocket);
            continue;
        }

		args->tunFd = tunFd;
        args->clientSocket = clientSocket;

		routingTable[clientIP] = clientSocket;

		pthread_t threadId;
        if (pthread_create(&threadId, NULL, handleClient, args) != 0) {
            perror("Failed to create thread");
            close(clientSocket);
            free(args);
        }

    }
	
	return 0;
}

int tunvpn_client(const char *server, int port) {
	// XXX: implement your client codes here ...
	
	fprintf(stderr, "## [client] starts ...\n");

	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0) errquit("Client Socket Error.. ");

	char serverIP[] = "172.28.28.";
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "172.28.28.1", &serverAddr.sin_addr); 

	int idx = 2;
    while(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		char buf[11];
		sprintf(buf, "%d", idx);
		strcat(serverIP, buf);
		inet_pton(AF_INET, serverIP, &serverAddr.sin_addr); 
		idx++;
		serverIP[10] = '\0';
    }

    unsigned int assignedIP;
    recv(clientSocket, &assignedIP, sizeof(assignedIP), 0);
    assignedIP = ntohl(assignedIP);

    char tunName[] = "tun0";
    int tunFd = tun_alloc(tunName);
    if(tunFd < 0) errquit("Tun0 Create Error..");

    ifreq_set_addr(clientSocket, tunName, htonl(assignedIP));
    ifreq_set_netmask(clientSocket, tunName, htonl(NETMASK));
    ifreq_set_flag(clientSocket, tunName, IFF_UP);

	while(1) {

		setNonBlocking(tunFd);
		setNonBlocking(clientSocket);

		char buffer[20480];
		int nread = read(tunFd, buffer, sizeof(buffer));
		if(nread > 0)
			write(clientSocket, buffer, nread);

		char buf[20480];
		int nnread = read(clientSocket, buf, sizeof(buf));
		write(tunFd, buf, nnread);
	}

	return 0;
}

int usage(const char *progname) {
	fprintf(stderr, "usage: %s {server|client} {options ...}\n"
		"# server mode:\n"
		"	%s server port\n"
		"# client mode:\n"
		"	%s client servername serverport\n",
		progname, progname, progname);
	return -1;
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		return usage(argv[0]);
	}
	if(strcmp(argv[1], "server") == 0) {
		if(argc < 3) return usage(argv[0]);
		return tunvpn_server(strtol(argv[2], NULL, 0));
	} else if(strcmp(argv[1], "client") == 0) {
		if(argc < 4) return usage(argv[0]);
		return tunvpn_client(argv[2], strtol(argv[3], NULL, 0));
	} else {
		fprintf(stderr , "## unknown mode %s\n", argv[1]);
	}
	return 0;
}
