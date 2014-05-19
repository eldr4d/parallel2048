#ifndef _CLIENT_COMM_H
#define _CLIENT_COMM_H

#include <time.h>
#include "Comm.hpp"
#include "Protocol.hpp"

void StartSession (int *sock, int side, int server_port, char *server_host);
void StartGame (int sock, MsgFromServer *msg);
void SendNormalAndGetPlacerMove (int sock, int dir, MsgFromServer *msg);
void SendPlacerAndGetNormalMove (int sock, int row, int col, int value, MsgFromServer *msg);
void EndSession (int sock);
void GetHost(int argc, char * argv[], char *host);
int GetPort(int argc, char * argv[]);
void GetMessageFromServer (int sock, MsgFromServer *msg);

#endif //_CLIENT_COMM_H

