#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using namespace std;
const int INF=1e9+7;
const int ms=51*51;
const int maxn=ms*5;
int N,ans[55],res[55];

struct DLX
{
    int n,id;
    int L[maxn],R[maxn],U[maxn],D[maxn];
    int C[maxn],S[maxn],loc[maxn][2];
    int H[ms];
    void init(int nn=0) 
    {
        n=nn;
        for(int i=0;i<=n;i++) U[i]=D[i]=i,L[i]=i-1,R[i]=i+1;
        L[0]=n; R[n]=0;
        id=n;
        memset(S,0,sizeof(S));
        memset(H,-1,sizeof(H));
    }
    void Link(int x,int y)
    {
        ++id;
        D[id]=y; U[id]=U[y];
        D[U[y]]=id; U[y]=id;
        loc[id][0]=x,loc[id][1]=y;
        C[id]=y; S[y]++;
        if(H[x]==-1) H[x]=L[id]=R[id]=id;
        else
        {
            int a=H[x];
            int b=R[a];
            L[id]=a; R[a]=id;
            R[id]=b; L[b]=id;
            H[x]=id;
        }
    }
    void Remove(int c)
    {
        L[R[c]]=L[c];
        R[L[c]]=R[c];
        for(int i=D[c];i!=c;i=D[i])
            for(int j=R[i];j!=i;j=R[j])
        {
            U[D[j]]=U[j];
            D[U[j]]=D[j];
            S[C[j]]--;
        }
    }
    void Resume(int c)
    {
        for(int i=U[c];i!=c;i=U[i])
            for(int j=R[i];j!=i;j=R[j])
        {
            S[C[j]]++;
            U[D[j]]=j;
            D[U[j]]=j;
        }
        L[R[c]]=c;
        R[L[c]]=c;
    }
    bool dfs(int step)
    {
        if(step>=N) return true;
        if(R[0]==0) return false;
        int Min=INF,c=-1;
        for(int i=R[0];i;i=R[i])
        {
            if(i>N) break;
            if(Min>S[i]){ Min=S[i]; c=i; }
        }
        if(c==-1) return false;
        Remove(c);
        for(int i=D[c];i!=c;i=D[i])
        {
            ans[step]=loc[i][0];
            for(int j=R[i];j!=i;j=R[j]) Remove(C[j]);
            if(dfs(step+1)) return true;
            for(int j=L[i];j!=i;j=L[j]) Resume(C[j]);
        }
        Resume(c);
        return false;
    }
}dlx;

bool vis[55*6];


int main() {
    const char* server_socket_path = "/queen.sock";

    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, server_socket_path, sizeof(server_address.sun_path));

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("connection");
        close(client_socket);
        return 1;
    }

    const char* message = "Start";
    if (send(client_socket, message, strlen(message), 0) == -1) {
        perror("Send");
        close(client_socket);
        return 1;
    }

    char response[1024];
    ssize_t bytes_received = recv(client_socket, response, sizeof(response), 0);
    if (bytes_received == -1) {
        perror("Receive");
    } else
        response[bytes_received] = '\0';

    int board[30][30];
    int res_id=0;
    int queenlist[30];
    memset(queenlist,-1,sizeof(queenlist));
    for(int i=0; i<30; i++){
        for(int j=0; j<30;j++){
            while((response[res_id]!='.')&&(response[res_id]!='Q')){
                res_id++;
            }
                
            if (response[res_id]=='.')
                board[i][j]=0;
            else if(response[res_id]=='Q'){
                board[i][j]=1;
                queenlist[i]=j;
            }
            res_id++;
        }
    }

    N = 30;
    dlx.init(N*6-2);
    memset(vis,false,sizeof(vis));
    int y;
    for(int i=1; i<=30;i++){
        int x = i;
        y = queenlist[i-1]+1;
        if(y==0) continue;
            int a=x,b=N+y,c=2*N+N+x-y,d=4*N+x+y-2; 
            vis[a]=vis[b]=vis[c]=vis[d]=true; 
            int t=(x-1)*N+y-1;
            dlx.Link(t,a); 
            dlx.Link(t,b);
            dlx.Link(t,c);
            dlx.Link(t,d);
    }

    for(int x=1;x<=N;x++)
        for(int y=1;y<=N;y++) {
			int a=x,b=N+y,c=2*N+N+x-y,d=4*N+x+y-2;
			if(vis[a]||vis[b]||vis[c]||vis[d]) 
				continue; 
			int t=(x-1)*N+y-1;
			dlx.Link(t,a);
			dlx.Link(t,b);
			dlx.Link(t,c);
			dlx.Link(t,d);
    	}

    if(!dlx.dfs(0)) 
		printf("No answer find\n");
    else
        for(int i=0;i<N;i++) 
			res[ans[i]/N]=ans[i]%N;

    memset(response,0,sizeof(response));

    for(int i = 0;i < 30; i++){
        string message1;
        if(queenlist[i]==-1){
            message1 = "M " + to_string(i) + " " + to_string(res[i]) + "\n";
            send(client_socket, message1.c_str(), message1.length(), 0);
            recv(client_socket, response, sizeof(response), 0);
        }
    }
    
    string message2 = "C\n";

    send(client_socket, message2.c_str(), message2.length(), 0);
    memset(response,0,sizeof(response));
    recv(client_socket, response, sizeof(response), 0);
	cout << response << endl;
	
    close(client_socket);
    return 0;
}
