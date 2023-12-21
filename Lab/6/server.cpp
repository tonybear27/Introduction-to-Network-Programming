#include "packet.hpp"
#define BUFFERSIE 1100

const int n = 2147493647;

int main(int argc, char *argv[]) {
	int s, port;
	string path;
	struct sockaddr_in sin, csin;
	socklen_t csinlen = sizeof(csin);

	path = string(argv[1]);
	sscanf(argv[3], "%d", &port);

	setvbuf(stdout, NULL, _IONBF, 0);
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) < 0)
		err_quit("buffer");

	if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0)
		err_quit("bind");
	
    string savePath = "";
    char mapp[SMG][400] = { "\0" };
    int length[SMG] = {0};

	while(1) {
        string header;
        bool flag = false;
        int len = 0;
        perFile rec;
        
        while(1) {
            memset(rec.header, '\0', sizeof(rec.header));
            memset(rec.packet, '\0', sizeof(rec.packet));

            len = recvfrom(s, &rec, sizeof(rec), 0, (struct sockaddr*) &csin, &csinlen);
            header = string(rec.header);

            if(header != "D") 
                break;

            if(!flag) {

                FILE* file = fopen(savePath.c_str(), "w");
                for(int i = 0; i < SMG; i++)
                    fwrite(mapp[i], 1, length[i], file);

                fclose(file);
                flag = 1;
            }
        }

        if(header == "AA")
            break;
        
        else if(header != "D") {   
            for(int i = 0; i < 5; i++)
                sendto(s, header.c_str(), 5, 0, (struct sockaddr*)&csin, csinlen);
            

            savePath = path + "/000" + header.substr(0, 3);
            string idx = header.substr(3, 2);
            int index = stoi(idx);

            for(int i = 0; i < rec.len; i++) 
                mapp[index][i] = rec.packet[i];
            
            mapp[index][rec.len] = '\0';
            length[index] = rec.len;
        }
	}


    char over[] = "XX";
	while(1) 
        sendto(s, over, sizeof(over), 0, (struct sockaddr*)&csin, csinlen);
	close(s);
	exit(0);
}