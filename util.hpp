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

int listen_inet_socket(int portnum);
void make_socket_non_blocking(int sockfd);
void perror_die(const std::string& msg);
// Dies (exits with a failure status) after printing the given printf-like
// message to stdout.
void die(const std::string& fmt, ...);
// Wraps malloc with error checking: dies if malloc fails.
void* xmalloc(size_t size);
void report_peer_connected(const struct sockaddr_in* sa, socklen_t salen);
