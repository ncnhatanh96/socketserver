#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <netdb.h>

int Start_ListenSocket(int portnum);
void Set_NONBlocking_Socket(int sockfd);
void perror_die(const std::string& msg);
void ReportClientConnection(const struct sockaddr_in* sa, socklen_t salen);
