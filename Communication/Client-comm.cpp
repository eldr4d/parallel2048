#include "Client-comm.hpp"


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
	if (hostname == NULL || !(hostname[0]))
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
    	std::cerr << "Socket not opened" << std::endl;
        exit (EXIT_FAILURE);
    }
    
    /* Connect to the server. */
    InitSockaddr (&servername, server_host, server_port);

    if (0 > connect (sock, (struct sockaddr *) &servername,
                     sizeof (servername))) {
    	std::cerr << "Connect failed: " << errno << std::endl;
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
}


void SendFirstMessage (int sock, int side)
{
    FirstMsgToServer to_msg;
    int nbytes;

    assert (side == 0 || side == 1);
    to_msg = side;
	std::cout << "Side =: " << (int)to_msg << std::endl;
    nbytes = write (sock, (char *)&to_msg, sizeof(FirstMsgToServer));
    if (nbytes < sizeof(FirstMsgToServer)) {
		std::cerr << "Exit smaller msg" << std::endl;
        exit (EXIT_FAILURE);
    }
	
    std::cout << "Sent first message to server side=" << side << std::endl;
}


void StartSession (int *sock, int side, int server_port, char *server_host)
{
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

