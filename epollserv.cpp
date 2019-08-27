#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50

void error_handling(char *);

int main(int argc, char *argv[])
{

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2)
    {
        printf("Usage : %s <IP>\n", argv[0]);
        exit(1);
    }

    //填写地址信息
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (-1 == (bind(serv_sock, (struct sockaddr *)&serv_addr,
                    sizeof(serv_addr))))
    {
        error_handling("bind() error...");
    }

    if (-1 == listen(serv_sock, 5))
    {
        error_handling("listen() error");
    }

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    // 设置时间关注度
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, 5000);
        if (event_cnt == -1)
        {
            error_handling("epoll exit... ");
        }

        if (event_cnt == 0)
        {
            printf("Timeout ....\n");
            continue;
        }

        for (i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock)
            {
                adr_sz = sizeof(clnt_addr);
                clnt_sock = accept(
                    serv_sock, (struct sockaddr *)&clnt_addr,
                    &adr_sz);

                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock,
                          &event);
                printf("connected clint: %d \n", clnt_sock);
            }
            else
            {
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if (str_len == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("closed client: %d \n", ep_events[i].data.fd);
                }
                else{
                    buf[str_len]=0;
                    printf("Get msg: %s \n", buf);
                    write(ep_events[i].data.fd, buf, BUF_SIZE);
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void error_handling(char *msg){
    printf("%s\n", msg);
    exit(1);
}