#include "Comm.hpp"


int make_socket (unsigned short int port)
{
    int sock;
    struct sockaddr_in name;

    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        exit (EXIT_FAILURE);
    }       

    /* Give the socket a name. */
    name.sin_family = AF_INET;
    name.sin_port = htons (port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
        exit (EXIT_FAILURE);
    }

    return sock;
}


void init_sockaddr (struct sockaddr_in *name, const char *hostname,
               unsigned short int port)
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

