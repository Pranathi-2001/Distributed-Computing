#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define PORT 62000
#define BUFFLEN 1024

char user[100];
char pwd[100];
char dir[200];
// char **dir_files;
char data[200];

char user_log[4][200];

void get_user(char commands[])
{
  char*cmd = strtok(commands," \n");
  strcpy(user,strtok(NULL," \n"));

 return;
}


void get_password(char commands[])
{
  
  char*cmd = strtok(commands," \n");
  strcpy(pwd,strtok(NULL," \n"));

 return;
}

void get_dir(char commands[])
{
  char*cmd = strtok(commands," \n");
  strcpy(dir,strtok(NULL," \n"));

 return;
}


int get_commands(char *pch,char commands[])
{
    if(strcmp(pch,"user")==0)

    {
      
      get_user(commands); //2
      return 2;
      
    }

    
    if(strcmp(pch,"pass")==0)
    {
      get_password(commands); //3
      
      return 3;
      
    }

    
    if(strcmp(pch,"cd")==0)
    {
      printf("%s\n",commands );
      get_dir(commands); //4
      
      return 4;
      
    }


    /*
    if(pch=="lcd")
    {
      if(check_user_pass(user,password)==1)
      {
        change_lcd(commands); //5
      }
      else 
        printf("User and password not matched cannot proceed\n");
      
    }*/

    
    if(strcmp(pch,"dir")==0)
    {
      
        printf("%s\n", commands);
        return 6;
    }

    
    if(pch=="get")
    {
        return 7;
    }

    
    if(pch=="put")
    {
        return 8;
    }

    
    if(pch=="mget")
    {
        return 9;
    }

    
    if(pch=="mput")
    {
    //   "User and password not matched cannot proceed\n");
        return 10;
    }
    
    
    return 0;
}


int check_user(FILE *fp,char*user)
{
    printf("%s\n", user);
    fp = fopen("user.txt","r");
    int i=0;
    while(fscanf(fp,"%200s",data)==1)
    {
        strcpy(user_log[i],data);
        i++;
    }
//printf("%s,%s,%s,%s\n",user_log[0],user_log[1],user_log[2],user_log[3]);

    if(strcmp(user,user_log[0])==0 )
        return 1;
    if(strcmp(user,user_log[2])==0 )
        return 1;
    return 0;

}


int check_pwd(char* pwd)
{

 // printf("%s,%s,%s,%s\n",user_log[0],user_log[1],user,pwd);
    if((strcmp(user,user_log[0])==0) && (strcmp(pwd,user_log[1])==0))
        return 1;
 
    if((strcmp(user,user_log[2])==0) && (strcmp(pwd,user_log[3])==0))
        return 1;
    return 0;
}

int main()
{
    FILE *fp ;
    fp = fopen("user.txt","r");
    int sockfd, clilen;//to create socket
    int newsockfd;//to accept connection
    struct sockaddr_in serv_addr;//server receive on this address
    struct sockaddr_in client_addr;//server sends to client on this address
    char* buff = (char*)calloc(sizeof(char),BUFFLEN);
	char* cmd_line = (char*)calloc(sizeof(char),BUFFLEN);

    int n;
    char msg[200];
    
    pid_t child;
    // socklen_t clilen;

    if((sockfd=socket(AF_INET, SOCK_STREAM,0))<0)
    {
        perror("[ERROR] Unable to create the TCP socket\n");
        exit(1);
    }

 
 //bind the socket with the server address and port
   

    bzero((char*) &serv_addr,sizeof(serv_addr));

    memset(&serv_addr,0,sizeof(serv_addr));
    memset(&client_addr,0,sizeof(client_addr));


    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(PORT);

    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[ERROR] Unable to bind the TCP socket\n");
        exit(1);
    }

    listen(sockfd,5);


    //parent process waiting to accept a new connection
    printf("\n*****server waiting for new client connection:*****\n");
    clilen = sizeof(client_addr);
        newsockfd=accept(sockfd,(struct sockaddr*)&client_addr,&clilen);
        if(newsockfd < 0){
			perror("Accept error");
			exit(1);
		}
    printf("connected to client\n");
    int count=0;
    while(1){
        
        // printf("Created Control channel\n");
        fflush(stdout);
        n=recv(newsockfd,msg,200,0);
        if(n < 0){
			perror("Receive error");
			exit(1);
		}
        else if(n==0){
			printf("Connection closed!!!\n");
			break;
		} 
        printf("Received command %s\n",msg);
        char cmds[200];
        strcpy(cmds,msg);

        char * pch ;
        pch = strtok(msg," \n");

        int m =get_commands(pch,cmds);
        printf("%d,%d\n",m,count);
        
    //for user
        if( (m==2) && (count==0))
        {
            int k=check_user(fp,user);
            if(k==1)
            {
                char* str="200";
                send(newsockfd,str,200,0);
                //printf("afa\n");
            }
            else
            {
            // printf("afsdaa\n");
                char* str="500";
                send(newsockfd,str,200,0);
            }
            count++;
        }

        else if((m==2) && (count!=0))
        {
            char* str="600";
            send(newsockfd,str,200,0);
        }
        
        //for password
        else if((m==3) && (count==1))
        {
            int k=check_pwd(pwd);
            if(k==1)
            {
                char* str="200";
                send(newsockfd,str,strlen(str),0);
                 count++;
            }
            else 
            {
                char* str="500";
                send(newsockfd,str,strlen(str),0);
                printf("Enter password again\n");
            }
           
        }

        else if((m==3) && (count<1))
        {
            char* str="600";
            send(newsockfd,str,strlen(str),0);
        }

        else  if((m==4) && (count>1))
        {
            char s[100];
            if(chdir(dir)!=0)
            {
                char* str="500";
                send(newsockfd,str,strlen(str),0);
            }

            else
            {
                char* str="200";
                send(newsockfd,str,strlen(str),0);
            }
        }
        else if(((m==4) && (count<1) ))
        {
            printf("first enter username and pasword\n");
            char* str="code 600\n";
            send(newsockfd,str,strlen(str),0);
        }
        else if(m==6){
            char* str="done with dir...\n";
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            if (d) {
                int i=0;
                printf("Sending the content of the directory...\n");
                while ((dir = readdir(d)) != NULL) {
                    // printf("%s\n", dir->d_name);
                    strcpy(data,dir->d_name);
                    send(newsockfd,data,sizeof(data),0);
                }
                closedir(d);
            }
            else{
                printf("error opening directory!!!\n");
            }
            strcpy(data,"\0");
            send(newsockfd,data,sizeof(data),0);
        }
        else if(m==7){
            char *file = strtok(cmds," ");
            file = strtok(NULL," ");
            int fd = open(file, O_RDONLY);
            int code[] = {0};
            if (fd==-1){
                perror("Error in opening file!!!\n");
                code[0] = htonl(500);
                send(newsockfd, code, sizeof(int), 0);
                continue;
            }
            else{
                code[0] = htonl(200);
                send(newsockfd, code, sizeof(int), 0);
                int nb = 16;
                // char buf2[BUFFLEN], buf[BUFFLEN];
                char numstr[20];
                char file_buff[10], buff[13], c[1];
                for(int i=0;i<13;i++)buff[i]='\0';
                while((nb=read(fd,file_buff,10))>0){
                    file_buff[nb]='\0';
                    if(nb==10)
                        c[0]='M';
                    else
                        c[0]='L';
                    int cnt=htonl(nb);
                    send(newsockfd, c,sizeof(c),0);
                    send(newsockfd,&cnt,sizeof(cnt),0);                  
                    send(newsockfd,file_buff,strlen(file_buff),0);
                    // If file reading is done then break
                    if(nb<10)
                        break;
                }
                close(fd);
            }
        }

        else if(m==8){
            char *file = strtok(cmds," ");
            file = strtok(NULL, " ");
            file = strtok(NULL, " ");
            int fd = open(file, O_CREAT | O_WRONLY |O_TRUNC, 0666);
            int code[] = {0};
            if (fd==-1){
                perror("Error in opening file!!!\n");
                code[0] = htonl(500);
                send(newsockfd, code, sizeof(int), 0);
                continue;
            }
            else{   
                code[0] = htonl(200);
                send(newsockfd, code, sizeof(int), 0);
                int l;
                char file_buff[10], buff[13], ch[1];
                int size1=0, p;
                    // File receiving module
                while(1){
                    char c[1];
                    int u = recv(newsockfd,c,sizeof(c),0);
                    c[u]='\0';
                    int p;
                    // Receive number of bytes
                    recv(newsockfd,&p,sizeof(int),0);
                    p=ntohl(p);
                    char buff[p];
                    // Receive content
                    p = recv(newsockfd,buff,sizeof(buff),0);
                    buff[p]='\0';
                    // Writing data to file
                    write(fd,buff,strlen(buff));
                    if(c[0]=='L')
                        break;
                }
                close(fd);
            }   
        }

        else if(m==9){
            int i=0; 
            int code[] = {0};
            for(i=0; i<2; i++){
                char *file = strtok(cmds," ");
                file = strtok(NULL,", ");
                if(i==1){
                    file = strtok(NULL," ");
                }
                int fd = open(file, O_RDONLY);
                if(fd == -1){
                    printf("Cannot open File(error)\n");
                    code[0] = htonl(500);
                    send(newsockfd, code, sizeof(int), 0);
                    continue;
                }
                else{
                    code[0] = htonl(200);
                    send(newsockfd, code, sizeof(int), 0);
                    int l;
                    char file_buff[10], buff[13], c[1];
                    for(int i=0;i<13;i++)buff[i]='\0';
                    while((l=read(fd,file_buff,10))>0){
                        file_buff[l]='\0';
                        if(l==10)
                            c[0]='M';
                        else
                            c[0]='L';
                        int cnt=htonl(l);
                        send(newsockfd,c,sizeof(c),0);
                        send(newsockfd,&cnt,sizeof(cnt),0);
                        send(newsockfd,file_buff,strlen(file_buff),0);
                        if(l<10)
                            break;
                    }
                    close(fd);
                    continue;
                }
            }
            // char *file = strtok(cmds," ");
            // file = strtok(NULL, " ");
            // char *file2 = strtok(NULL, " ");
        }

        else if(m==10){
            // char *file = strtok(cmds," ");
            // file = strtok(NULL, " ");
            // char *file2 = strtok(NULL, " ");
            int i=0; 
            int code[] = {0};
            for(i=0; i<2; i++){
                char *file = strtok(cmds," ");
                file = strtok(NULL,", ");
                if(i==1){
                    file = strtok(NULL," ");
                }
                int fd = open(file, O_RDONLY);
                if(fd == -1){
                    printf("Cannot open File(error!!)\n");
                    code[0] = htonl(500);
                    send(newsockfd, code, sizeof(int), 0);
                    continue;
                }
                else{
                    printf("Yes..\n");
                    code[0] = htonl(200);
                    send(newsockfd, code, sizeof(int), 0);
                    int l, p;
                    while(1){
                        char c[1];
                        int u = recv(newsockfd,c,sizeof(c),0);
                        c[u]='\0';
                        recv(newsockfd,&p,sizeof(int),0);//number of bytes
                        p=ntohl(p);
                        char buff[p];
                        p = recv(newsockfd,buff,sizeof(buff),0); // Receive buffer
                        buff[p]='\0';
                        // Writing data to file
                        write(fd,buff,strlen(buff));
                        if(c[0]=='L')
                            break;
                    }
                    close(fd);
                    continue;
                }
            }
        }

        if(n==0)
        {
            close(newsockfd);
            break;
        }
    }
    close(sockfd);
    return 0;
}