#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include<errno.h> 


#define BUFFLEN 1024
extern int errno; 


char* ip_addr;
char* port_no_cmd;
char* dir;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main()
{
	FILE *fp ;
	fp = fopen("user.txt","r");
   	int sockfd,newsockfd,portno,n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	

	int i;
	char buf[200];	

   	char *reply = (char *) calloc(200, sizeof(char));

	//execution of open command seperately as it should be the first command in client

	while(1)
	{
	    printf("myFTP> ");
		fgets(buf,200,stdin);
        // cout<<buf<<endl;
		printf("%s",buf);
		
		char* cmd = strtok(buf," ");
   		ip_addr= strtok(NULL," ");
   		port_no_cmd=strtok(NULL," ");

		if(strcmp(cmd,"open")!=0)
		{
			printf("\nfirst command should be open\n");
		}
		else
		{			     
            portno=atoi(port_no_cmd);
            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
            {
                perror("Cannot create socket\n");
                exit(0);
            }		
            server=gethostbyname(ip_addr);
            if(server==NULL)
            {
                fprintf(stderr, "No such host\n");
            }
            bzero((char*)&serv_addr , sizeof(serv_addr));

            serv_addr.sin_family  = AF_INET;
            inet_aton(ip_addr, &serv_addr.sin_addr);
            serv_addr.sin_port  = htons(portno);
            bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);



            if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) 
            {
                perror("Unable to connect to server\n");
                exit(0);
            }

            else 
            {
                printf("created successfully\n");
            }
      		break;
		}
	}

	bzero(buf,200);

	while(1)
	{
		printf("newmyFTP> ");
		
		char cmds[200];
		fgets(buf,200,stdin);
        strcpy(cmds,buf);
        
        char * pch ;
        pch = strtok(buf," \n");
		// printf("%s",cmds);
		//printf("%d",strcmp(cmds,"quit"));
		if(strcmp(pch,"quit")==0)
		{
            // close(sockfd);
			break;
		}
		
		if(strcmp(pch,"lcd")==0)
		{
			char s[100];
			char*cmd=strtok(cmds," ");
			dir=strtok(NULL," \n");
			chdir(dir);
		}
		 
		else{
            // printf("check...");
            n=send(sockfd,cmds,strlen(cmds),0);
            if(strcmp(pch, "dir")==0){
                while(1){
                    char *str = "done";
                    if(strcmp(reply, "\0") == 0){
                        break;
                    }
                    n=recv(sockfd,reply,200,0);
                    printf("%s\n", reply);
                }
                continue;
            }
            if(strcmp(pch, "get") == 0){
                char *file=strtok(cmds," ");
                file=strtok(NULL," ");
                file=strtok(NULL," ");
                printf("Get command...\n");
                int fd = open(file, O_CREAT | O_WRONLY |O_TRUNC, 0666);
                if(fd==-1){
                    printf("File %s not found\n",file);
                    continue;
                }

                int code[]={0};
				int n = recv(sockfd, code, sizeof(int), 0);
                code[0] = ntohl(code[0]);
                if(code[0] == 500){
                    printf("code: 500, File can't be opened(doesn't exits)!!");
                    continue;
                }
                else{   
                    // printf("Yes..");
                    char c[1];
                    int p;
                    while(1){
                        int x = recv(sockfd,c,sizeof(c),0);
                        c[x]='\0';
                        recv(sockfd,&p,sizeof(int),0);
                        p=ntohl(p);
                        char buff[p];   
                        // content in buffer
                        p = recv(sockfd,buff,sizeof(buff),0);
                        buff[p]='\0';
                        write(fd,buff,strlen(buff));
                        if(c[0]=='L')
                            break;
                    }
                    continue;
                }
            }
            if(strcmp(pch, "put") == 0){
                char *file = strtok(cmds," ");
                file = strtok(NULL," ");
                int fd = open(file, O_RDONLY);
                if(fd == -1){
                    printf("Cannot open file\n");
                    continue;
                }
                int code[]={0};
				int n = recv(sockfd, code, sizeof(int), 0);
                code[0] = ntohl(code[0]);
                if(code[0] == 500){
                    printf("code: 500, File can't be opened!!");
                    continue;
                }
                else if(code[0] = 200){
                    int nb = 16;
                    char buf2[10], buf[1];
                    char numstr[17];
                    while (nb = read(fd, buf2, 10) > 0){
                        // bzero(buf, sizeof(buf));
                        buf2[nb] = '\0';
                        if(nb==10)
                            buf[0] =  'M';
                        else
                            buf[0] = 'L';
                        // numstr = (nb);
                        int cnt = htonl(nb);
                        send(sockfd, buf, sizeof(buf), 0);
                        send(sockfd, &cnt, sizeof(buf2) + 1, 0);
                        send(sockfd, buf2, strlen(buf2) + 1, 0);
                        // bzero(buf, sizeof(buf));
                        if(nb < 10) break;
                    }
                    close(fd);
                    continue;
                }
            }
            if(strcmp(pch, "mget") == 0){
                int i=0;
                for(i=0; i<2; i++){
                    char *file = strtok(cmds," ");
                    file = strtok(NULL,", ");
                    if(i==1){
                        file = strtok(NULL," ");
                    }
                    int fd = open(file, O_RDONLY);
                    if(fd == -1){
                        printf("Cannot open file\n");
                        continue;
                    }
                    int code[]={0};
                    int n = recv(sockfd, code, sizeof(int), 0);
                    code[0] = ntohl(code[0]);
                    if(code[0] == 500){
                        printf("code: 500, File can't be opened(doesn't exists!!");
                        continue;
                    }
                    else{
                        char c[1];
                        int p;
                        while(1){
                            int u = recv(sockfd,c,sizeof(c),0);
                            c[u]='\0';
                            recv(sockfd,&p,sizeof(int),0);
                            p=ntohl(p);
                            char buff[p];
                            // Receive buffer
                            p = recv(sockfd,buff,sizeof(buff),0);
                            buff[p]='\0';
                            write(fd,buff,strlen(buff));
                            if(c[0]=='L')
                                break;
                        }
                        close(fd);
                        continue;
                    }
                }
            }
            if(strcmp(pch, "mput") == 0){
                int i=0;
                for(i=0; i<2; i++){
                    char *file = strtok(cmds," ");
                    file = strtok(NULL,", ");
                    if(i==1){
                        file = strtok(NULL," ");
                    }
                    int fd = open(file, O_RDONLY);
                    if(fd == -1){
                        printf("Cannot open file\n");
                        continue;
                    }
                    int code[]={0};
                    int n = recv(sockfd, code, sizeof(int), 0);
                    code[0] = ntohl(code[0]);
                    if(code[0] == 500){
                        printf("code: 500, File can't be opened(doesn't exists!!");
                        continue;
                    }
                    else{
                        char buff[10];
                        int nb;
                        char c[1];
                        // File sending module
                        while((nb=read(fd,buff,10))>0){
                            buff[nb]='\0';
                            if(nb==10)
                                c[0]='M';
                            else
                                c[0]='L';
                            int cnt=htonl(nb);
                            send(sockfd,c,sizeof(c),0);
                            send(sockfd,&cnt,sizeof(cnt),0); // Send number of bytes
                            send(sockfd,buff,strlen(buff),0);
                            if(nb<10)
                                break;
                        }	
                        close(fd);
                        continue;
                    }
                }
            }
            n=recv(sockfd,reply,200,0);
            printf("%s\n",reply);
            n=recv(sockfd,reply,200,0);
            errno=atoi(reply);
            printf("%d\n",errno);
            if(errno==100)
            {
                printf("No such command exists\n");
            }
		}
		
	}
    close(sockfd);
    return 0;
}