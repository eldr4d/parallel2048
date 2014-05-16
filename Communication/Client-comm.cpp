#include "Client-comm.hpp"


/* To be used for time measurements - do not change */
static clock_t end_of_time; 
static clock_t move_start; 

void GetHost(int argc, char * argv[], char *host)
{
	if (argc < 4) 
		strcpy(host, "localhost");
	else 
		strcpy(host, argv[3]);
}


int GetPort(int argc, char * argv[])
{
	if (argc < 3)
		return PORT;
	return atoi(argv[2]);
}

void InitSockaddr (struct sockaddr_in *name, const char *hostname, unsigned short int port)
{   
	struct hostent *hostinfo;
    
	name->sin_family = AF_INET;
	name->sin_port = htons (port);
    
	/* If hostname is not defined, assume the server is on my machine */
	if (hostname == NULL || hostname[0] == NULL)
		hostinfo = gethostbyname ("localhost");
	else 
		hostinfo = gethostbyname (hostname);
	if (hostinfo == NULL) {
		std::cerr << "Unknown host " << hostname << "." << std::endl;
		exit (EXIT_FAILURE);
	}
    
	name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}       


int InitClientComm (int server_port, char *server_host)
{
    int sock;
    struct sockaddr_in servername;

    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        exit (EXIT_FAILURE);
    }
    
    /* Connect to the server. */
    InitSockaddr (&servername, server_host, server_port);

    if (0 > connect (sock, (struct sockaddr *) &servername,
                     sizeof (servername))) {
        exit (EXIT_FAILURE);
    }
    return sock;
}




void GetMessageFromServer (int sock, MsgFromServer *msg)
{
    int nbytes;

    nbytes = read (sock, (char *)msg, sizeof(MsgFromServer));
    if (nbytes < sizeof(MsgFromServer)) {
        exit (EXIT_FAILURE);
    }


    move_start = clock();
    end_of_time = move_start + (clock_t)(msg->time_left * CLOCKS_PER_SEC);
    if (end_of_time < move_start) {
       std::cerr << "clock() may wrap around during this game" << std::endl;
    }
}


void SendFirstMessage (int sock, int side)
{
    FirstMsgToServer to_msg;
    int nbytes;

    assert (side == 0 || side == 1);
    to_msg = side;
	std::cout << "Side =: " << to_msg << std::endl;
    nbytes = write (sock, (char *)&to_msg, sizeof(FirstMsgToServer));
    if (nbytes < sizeof(FirstMsgToServer)) {

        exit (EXIT_FAILURE);
    }
	
    std::cout << "Sent first message to server side=" << side << std::endl;
}


void StartSession (int *sock, int side, int server_port, char *server_host)
{
    int ret;

    ret = clock();  /* Start CPU clock counter. */
    if (ret == (clock_t) -1) {
        std::cerr << "clock() failed" << std::endl;
        exit (EXIT_FAILURE);
    }

    *sock = InitClientComm(server_port, server_host);
    SendFirstMessage (*sock, side);
}


void StartGame (int sock, MsgFromServer *msg)
{
    GetMessageFromServer (sock, msg);
}


void SendPlacerAndGetNormalMove (int sock, int row, int col, int value, MsgFromServer *msg)
{
  	MsgToServer msgToServ;
  	msgToServ.two = value == 2? false : true;
  	msgToServ.row = row;
  	msgToServ.col = col;
  	msgToServ.pl = PLACER;
    int nbytes;
    double time_left;


    nbytes = write (sock, (char *)&msgToServ, sizeof(MsgToServer));
    if (nbytes < sizeof(MsgToServer)) {
        exit (EXIT_FAILURE);
    }

    GetMessageFromServer (sock, msg);
}


void SendNormalAndGetPlacerMove (int sock, int dir, MsgFromServer *msg)
{
  	MsgToServer msgToServ;

  	msgToServ.dir = dir;
  	msgToServ.pl = NORMAL;
    int nbytes;
    double time_left;


    nbytes = write (sock, (char *)&msgToServ, sizeof(MsgToServer));
    if (nbytes < sizeof(MsgToServer)) {
        exit (EXIT_FAILURE);
    }

    GetMessageFromServer (sock, msg);
}

void EndSession (int sock)
{
    close (sock);
}

