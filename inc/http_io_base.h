
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <errno.h>

// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <sys/epoll.h>
// #include <unistd.h>
// #include <sys/types.h>
#include "sock.h"

#define FDSIZE 1000
#define EPOLLEVENTS 100

static int socket_bind(const char *ip, int port);

int http_socket_bind(const char *ip, int port) { return socket_bind(ip, port); }

static void do_epoll(int listenfd);

int http_do_epoll(int listenfd) { return do_epoll(listenfd); }

static void handle_events(int epollfd, struct epoll_event *events, int num,
                          int listenfd, char *buf);
int http_handle_events(int epollfd, struct epoll_event *events, int num,
                       int listenfd, char *buf) {
  return handle_events(epollfd, events, num, listenfd, buf);
}

static void handle_accpet(int epollfd, int listenfd);
int http_handle_accpet(int epollfd, int listenfd) {
  return handle_accpet(epollfd, listenfd);
}

static void do_read(int epollfd, int fd, char *buf);

void http_do_read(int epollfd, int fd, char *buf) {
            return epollfd,fd,buf);
}

static void do_write(int epollfd, int fd, char *buf);
void http_do_write(int epollfd, int fd, char *buf) {
            return epollfd,fd,buf);
}

static void add_event(int epollfd, int fd, int state);
void http_add_event(int epollfd, int fd, int state) {
  return add_event(epoll, fd, state);
}

static void modify_event(int epollfd, int fd, int state);

void http_modify_event(int epollfd, int fd, int state) {
  return modify_event(epollfd, fd, state);
}

/*@http_delete_event start@*/
static void delete_event(int epollfd, int fd, int state);

/*@http_delete_event start@*/
void http_delete_event(int epollfd, int fd, int state) {
  return delete_event(epollfd, fd, state);
}
/*@http_delete_event end @*/

int main(int argc,char *argv[])
{
  int  listenfd;
     listenfd = open_listenfd("","9090");
     do_epoll(listenfd);
     return 0;
 }

static int socket_bind(const char *ip, int port) {
  int listenfd;
  struct sockaddr_in servaddr;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("socket error:");
    exit(1);
  }
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &servaddr.sin_addr);
  servaddr.sin_port = htons(port);
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind error: ");
    exit(1);
  }
  return listenfd;
}

static void do_epoll(int listenfd) {
  int epollfd;
  struct epoll_event events[EPOLLEVENTS];
  int ret;
  char buf[MAXSIZE];
  memset(buf, 0, MAXSIZE);
  //创建一个描述符
  epollfd = epoll_create(FDSIZE);
  //添加监听描述符事件
  add_event(epollfd, listenfd, EPOLLIN);
  for (;;) {
    //获取已经准备好的描述符事件
    ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
    handle_events(epollfd, events, ret, listenfd, buf);
  }
  close(epollfd);
}

static void handle_events(int epollfd, struct epoll_event *events, int num,
                          int listenfd, char *buf) {
  int i;
  int fd;
  //进行选好遍历
  for (i = 0; i < num; i++) {
    fd = events[i].data.fd;
    //根据描述符的类型和事件类型进行处理
    if ((fd == listenfd) && (events[i].events & EPOLLIN))
      handle_accpet(epollfd, listenfd);
    else if (events[i].events & EPOLLIN)
      do_read(epollfd, fd, buf);
    else if (events[i].events & EPOLLOUT)
      do_write(epollfd, fd, buf);
  }
}
static void handle_accpet(int epollfd, int listenfd) {
  int clifd;
  struct sockaddr_in cliaddr;
  socklen_t cliaddrlen;
  clifd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddrlen);
  if (clifd == -1)
    perror("accpet error:");
  else {
    printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr),
           cliaddr.sin_port);
    //添加一个客户描述符和事件
    add_event(epollfd, clifd, EPOLLIN);
  }
}

static void do_read(int epollfd, int fd, char *buf) {
  int nread;
  nread = read(fd, buf, MAXSIZE);
  if (nread == -1) {
    perror("read error:");
    close(fd);
    delete_event(epollfd, fd, EPOLLIN);
  } else if (nread == 0) {
    fprintf(stderr, "client close.\n");
    close(fd);
    delete_event(epollfd, fd, EPOLLIN);
  } else {
    printf("read message is : %s", buf);
    //修改描述符对应的事件，由读改为写
    modify_event(epollfd, fd, EPOLLOUT);
  }
}

static void do_write(int epollfd, int fd, char *buf) {
  int nwrite;
  nwrite = write(fd, buf, strlen(buf));
  if (nwrite == -1) {
    perror("write error:");
    close(fd);
    delete_event(epollfd, fd, EPOLLOUT);
  } else
    modify_event(epollfd, fd, EPOLLIN);
  memset(buf, 0, MAXSIZE);
}

static void add_event(int epollfd, int fd, int state) {
  struct epoll_event ev;
  ev.events = state;
  ev.data.fd = fd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state) {
  struct epoll_event ev;
  ev.events = state;
  ev.data.fd = fd;
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void modify_event(int epollfd, int fd, int state) {
  struct epoll_event ev;
  ev.events = state;
  ev.data.fd = fd;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
