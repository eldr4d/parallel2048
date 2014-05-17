#ifndef _COMM_H
#define _COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>


#define PORT    5555
#define MAXMSG  512


/* Prototypes for comm.h */

int make_socket(unsigned short int port);
void init_sockaddr (struct sockaddr_in *name, const char *hostname,
               unsigned short int port);

#endif //_COMM_H

