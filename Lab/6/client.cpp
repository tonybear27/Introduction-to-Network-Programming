#include "packet.hpp"

void sendFile(int start, int end, int32_t port, char path_to_file[]) {

    map<string, bool> datamap;
    socklen_t clilen;

    int connFD = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in	servaddr;
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
    int csize = sizeof(servaddr);

    int flags = fcntl(connFD, F_GETFL, 0);
    fcntl(connFD, F_SETFL, flags | O_NONBLOCK);

    for (int f_seq = start; f_seq < end; f_seq++) {
        
        datamap.clear();

        /*** Packet to send ***/
        perFile pkt[SMG];
        
        /*** Read file ***/
        char filename[20] = "\0", data[MAXLINE] = "\0";
        sprintf(filename, "%s/%06d", path_to_file, f_seq);
        FILE* inputFD = fopen(filename, "rb");
        rewind(inputFD);
        size_t num_data = fread(data, 1, MAXLINE, inputFD);
        data[num_data] = '\0';

        for(int i = 0; i < SMG; i++) {
            int idx = 0;
            sprintf(pkt[i].header, "%03d%02d", f_seq, i);
            for(int j = i * num_data / SMG; j < (i + 1) * num_data / SMG; j++) {
                pkt[i].packet[idx++] = data[j];
                pkt[i].len++;
            }
            pkt[i].packet[idx] = '\0';
        }

        /*** Send Packet ***/
        while (1) {

            bool all_set = true;
                
            char serv_reply[10] = {};

            for (int i_seg = 0; i_seg < SMG; i_seg++) {
                
                /*** Skip if received ACK ***/
                sprintf(serv_reply, "%03d%02d", f_seq, i_seg);
                string map_idx(serv_reply);
                if (datamap.count(map_idx) && datamap[map_idx])
                    continue;

                if (!datamap.count(map_idx))
                    datamap[map_idx] = false;
                
                all_set = false;

                sendto(connFD, &pkt[i_seg], sizeof(pkt[i_seg]), 0, (struct sockaddr*)&servaddr, csize);
                
                memset(serv_reply, '\0', sizeof(serv_reply));
                while(recvfrom(connFD, serv_reply, 5, 0, (struct sockaddr*)&servaddr, &clilen) == 5) {
                    serv_reply[6] = '\0';
                    string servac(serv_reply);
                    datamap[servac] = true;
                }

                usleep(90);
            }

            if (all_set) {
                perFile temp; 
                strcpy(temp.header, "D");
                strcpy(temp.packet, "");

                for (int i = 0; i < 5; i++) 
                    sendto(connFD, &temp, sizeof(temp), 0, (struct sockaddr*)&servaddr, csize);
                
                break;
            }
            
        }
    }


    while(1) {
        perFile temp;
        char rec[10] = "\0";

        strcpy(temp.header, "AA");
        strcpy(temp.packet, "AA");

        for (int i = 0; i < 5; i++) 
            sendto(connFD, &temp, sizeof(temp), 0, (struct sockaddr*)&servaddr, csize);
        
        int k = recvfrom(connFD, rec, sizeof(rec), 0, (struct sockaddr*)&servaddr, &clilen);
        if(k == 3)
            break;

    }
}

int main(int argc, char **argv) {

    char servip[20] = {}, path_to_file[20] = {};
    int	total;
    int32_t port;

	setvbuf(stdin, NULL, _IONBF, 0);

    if (argc >= 5) {
        strcpy(path_to_file, argv[1]);
        total = atoi(argv[2]);
        port = atoi(argv[3]);
        strcpy(servip, argv[4]);
    }

    sendFile(0, 1000, port, path_to_file);
    
    return 0;
}