#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#define BUF_SIZE 100
using namespace std;

void error_handling(char *);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    socklen_t adr_sz;

    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if (argc != 2)
    {
        printf("Usage : %s <IP>\n");
        exit(1);
    }

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

    if (-1 == bind(serv_sock,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        error_handling("bind() error...");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_handling("listen() error");
    }

    FD_ZERO(&reads);
    cout << "向fd_set变量reads注册套接字：serv_sock = " << serv_sock << endl;
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;
    cout << "fd_max : " << fd_max << endl;

    while (1)
    {

        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
            break; //IO出错
        if (fd_num == 0){
            // cout<<"Time-out"<<endl;
            continue; // 无任何变化
        }
        for (i = 0; i < fd_max + 1; i++)
        {
            cout << "进入循环： i ------>>>>>>>>>  " << i << endl;
            if (FD_ISSET(i, &cpy_reads))
            {
                if (i == serv_sock)
                { // connection...
                    cout << "start connection ... " << i << endl;
                    adr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock,
                                       (struct sockaddr *)&clnt_addr, &adr_sz);

                    cout << "conection.... clntsock = " << clnt_sock << endl;

                    FD_SET(clnt_sock, &reads);

                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    cout << "new fd_max : " << fd_max << endl;
                    cout<<"clnt_sock connect :"<<clnt_sock<<endl;
                    printf("connection from %s \n", inet_ntoa(clnt_addr.sin_addr));
                }
                else
                {
                    str_len = read(i, buf, BUF_SIZE);
                    if (str_len == 0)
                    {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("close...client %d \n", i);
                    }
                    else
                    {   
                        buf[str_len] =0;
                        cout<<"Get : "<<buf<<endl;
                        write(i, buf, str_len);
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *msg)
{
    cout << msg << endl;
    exit(1);
}