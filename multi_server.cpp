#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#define BUFF_SIZE 30

using namespace std;

void error_hanlding(char *);
void read_childPro(int);

int main(int argc, char *argv[])
{

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;

    pid_t pid;
    struct sigaction act;
    socklen_t clnt_addr_sz;
    int str_len, state;

    char buf[BUFF_SIZE];

    if (argc != 2)
    {
        cout << "usage :" << argv[0] << " <PORT>" << endl;
        exit(1);
    }

    //创建一个记录子线程
    int fds[2];
    pipe(fds);
    pid = fork();
    if(pid == 0){
        printf("Create a log proc.\n");
        FILE * fp = fopen("log.txt","wt");
        char msgBUFF[BUFF_SIZE];
        int i ,len;
        for(i =0 ;i< 10;i++){
            printf("id %d \n", i);
            len = read(fds[0], msgBUFF, BUFF_SIZE);
            printf("msg %s \n", msgBUFF);

            msgBUFF[len]=0;
            printf("id %d ", i);
            if (!strcmp(msgBUFF,"EOF")){
                printf("file close.");
                break;
                }
            fwrite(msgBUFF, 1, len, fp);

        }
        fclose(fp);
        return 0;


    }


    //注册信号函数
    act.sa_handler = read_childPro;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    state = sigaction(SIGCHLD, &act, 0);

    //填入地址信息
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // time_wait
    int option;
    socklen_t optlen = sizeof(option);
    option = 1;
    setsockopt(serv_sock, SOL_SOCKET,
               SO_REUSEADDR, (void *)&option, optlen);
    

    cout<<"serv_sock: "<<serv_sock<<endl;

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_hanlding("bind() error");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_hanlding("listen() error");
    }

    


    while (1)
    {

        socklen_t adr_sz = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &adr_sz);
        cout<<"clnt _sock : "<<clnt_sock<<endl;
        if (clnt_sock == -1)
        {
            continue;
        }
        else
        {
            char *addr = inet_ntoa(clnt_addr.sin_addr);
            printf("new client connected from %s .\n",
                   addr);
        }

        //创建子进程
        pid = fork();
        if (pid == -1)
        {
            cout << "Create child proc failded...\n" << endl;
            close(clnt_sock);
        }
        if (pid == 0)
        {
            string msg_recv="";
            //子进程程序
            while ((str_len = read(clnt_sock, buf, BUFF_SIZE - 1)) != 0)
            {
                write(clnt_sock, buf, str_len);
                buf[str_len] = 0;
                printf("%s\n", buf);
                write(fds[1], buf, str_len);
            }
          
            close(clnt_sock);
            close(serv_sock);
            cout << "client disconection...\n" << endl;
            return 0;
        }
        else
        {
            close(clnt_sock);
        }
    }

    write(fds[0], "EOF", 4);
    sleep(0.5);
    close(serv_sock);

    return 0;
}

void read_childPro(int sig)
{
    pid_t pid;
    int status;

    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}

void error_hanlding(char *s)
{
    cout << s << endl;
    exit(1);
}
