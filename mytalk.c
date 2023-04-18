#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "sethstd.h"

#define verb(action) if(verbose){action;}
#define GOMESSAGE "ok"
#define NOMESSAGE "no"
#define MID "@"

const int LEN = 100;
const int DEFAULT_BACKLOG = 100;

void doHost(int portNum, int wind, int barge, int verbose);
void doClient(int portNum, const char* hostname, int wind, int verbose);
uint32_t getaddress(const char *hostname);
void communicate(int sock);
int sendConfirmation(int sock, const char* hostname);
int recvConfirmation(int sock, int autoConf, struct in_addr addr);

int main(int argc, char** argv){
    
    int opt;
    int verbose = 0;
    int barge = 0;
    int noWind = 0;
    int port;
    char* hostname;

    /*parse args*/
    while((opt = getopt(argc, argv, "vaN")) != -1){
        switch(opt){
            case 'v':
                verbose += 1;
                break;
            case 'a':
                barge = 1;
                break;
            case 'N':
                noWind = 1;
                break;
            case '?': 
                perror("unkown option");
                exit(1);
                break;
        }
    }

    set_verbosity(verbose);

    /*determine server vs client*/
    switch(argc-optind){
        case 1:
            /*get port and set up server*/
            port = atoi(argv[optind]);
            doHost(port, !noWind, barge, verbose);
            break;

        case 2:
            /*get port an server hostname and set up client*/
            hostname = argv[optind];
            port = atoi(argv[optind+1]);
            doClient(port, hostname, !noWind, verbose);
            break;

        default:
        perror("usage: mytalk [ -v ] [ -a ] [ -N ] [ hostname ] port");
        exit(1);
        break;
    }

    /*close any open windows*/
    stop_windowing();

    return 0;
}

/*
doHost takes a portNum andarguments
then sets up a locally hosted server
on the given port number. Uses TCP.
*/
void doHost(int portNum, int wind, int barge, int verbose){
    
    struct sockaddr_in sa, newsa, peerinfo;
    int newsock;
    int sock = errNum(socket(AF_INET,SOCK_STREAM,0),"failed to make socket",1);
    socklen_t len;

    verb(printf("setting host. port num: %d\n", portNum));

    /*initialize sock address*/
    sa.sin_family = AF_INET;
    sa.sin_port = htons(portNum);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    /*bind socket to address*/
    verb(printf("binding with address: %d\n",sa.sin_addr.s_addr));
    errNum(bind(sock, (struct sockaddr*)&sa, sizeof(sa)), "failed to bind", 1);

    /*listen to socket for connection*/
    verb(printf("listening on port %d, address: %d\n",
        sa.sin_port,sa.sin_addr.s_addr));
    errNum(listen(sock, DEFAULT_BACKLOG), "failed to listen", 1);

    len = sizeof(sa);

    /*accept connection on listening socket*/
    verb(printf("waiting for accept on port %d, address: %d\n",
        sa.sin_port,sa.sin_addr.s_addr));
    newsock = errNum(accept(sock, (struct sockaddr*)&peerinfo, &len),
        "failed to accept",1);
    
    /*confirm with client and user that communications should be establishd*/
    if(recvConfirmation(newsock, barge, peerinfo.sin_addr)){

        verb(printf("opening communications\n"));

        /*open window if desired*/
        if(wind){
            verb(printf("starting window\n"));
            start_windowing();
        }
        
        /*communicate with client*/
        communicate(newsock);
    }
    close(sock);
    close(newsock);
    return;
}

/*
doClient takes a portNum and hostname
associated with that portNum and attempts
to establish a TCP connection with that
host, on that port.
*/
void doClient(int portNum, const char* hostname, int wind, int verbose){
    
    struct sockaddr_in sockAdr;
    int sock = errNum(socket(AF_INET,SOCK_STREAM,0),"failed to make socket",2);
    
    verb(printf("setting client. port num: %d, hostname: %s\n", 
        portNum, hostname));

    /*initialize sock address*/
    memset(&sockAdr,0,sizeof(sockAdr));
    sockAdr.sin_family = AF_INET;
    sockAdr.sin_port = htons(portNum);
    sockAdr.sin_addr.s_addr = (getaddress("unix5.csc.calpoly.edu"));

    /*connect to server*/
    verb(printf("connecting on port %d address: %d\n",
        sockAdr.sin_port, sockAdr.sin_addr.s_addr));
    errNum(connect(sock, (struct sockaddr*)&sockAdr, sizeof(sockAdr)), 
        "failed to connect",2);

    /*verify that connection is wanted*/
    if(sendConfirmation(sock, hostname)){

        verb(printf("opening communications\n"));

        /*open window if desired*/
        if(wind){
            verb(printf("starting window\n"));
            start_windowing();
        }

        /*communicate with server*/
        communicate(sock);
    }
    else{
        printf("%s declined connection.\n", hostname);
    }
    close(sock);
    return;
}

/*
getaddress is a given wrapper
function for getaddrinfo. Given
a hostname getaddress will return
the address for that host
*/
uint32_t getaddress(const char *hostname) {
    /* return the IPv4 address for the given host, or 0
    * Address is in network order */ 
    struct addrinfo *ai, hints;
    uint32_t res = 0;
    int rvalue;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;

    if ( 0 == (rvalue=getaddrinfo(hostname,NULL,&hints,&ai))) {
        if ( ai )
            res = ((struct sockaddr_in*)ai->ai_addr)->sin_addr.s_addr;
            freeaddrinfo(ai);
        } else {
            fprintf(stderr,"%s:%s\n", hostname, gai_strerror(rvalue));
    }
    return res; 
}

/*
communicate sends and recieves
messages across the given, properly
bound and connected, socket.
Communication continues until either
server or client terminates communications
*/
void communicate(int sock){

    char* buffer = scalloc(LEN, sizeof(char), "failed to initialize buffer",3);
    int recvSize = 1;
    int read;

    struct pollfd pfd[2];

    /*initialize pfd*/
    pfd[0].fd = sock;
    pfd[0].events = POLLIN;
    pfd[1].fd = STDIN_FILENO;
    pfd[1].events = POLLIN;

    do{
        /*poll file descriptors until they are ready to read*/
        errNum(poll(pfd,sizeof(pfd)/sizeof(struct pollfd),-1), 
            "polling failed",3);

        /*check which file descriptor is ready*/
        if(pfd[1].revents == POLLIN){

            update_input_buffer();

            /*once the internal buffer has a full line, send it to the peer*/
            if(has_whole_line()){
                read = read_from_input(buffer, LEN);
                errNum(send(sock, buffer, read, 0),"failed to send message",3);
            }
        }
        if(pfd[0].revents == POLLIN){

            /*recieve message from peer and write it to output*/
            recvSize = errNum(recv(sock, buffer, LEN, 0), 
                "failed to recieve message",3);
            write_to_output(buffer, recvSize);
        }

    }while(!has_hit_eof() && recvSize != 0);
    
    free(buffer);
    return;
}

/*
sendConfirmation sends a packet from
the client to the server containing
the client's name, then waits for
and verifies a response message from
the server
*/
int sendConfirmation(int sock, const char* hostname){
    char* buffer = scalloc(LEN, sizeof(char), 
        "failed to intialize confirmation buffer", 4);
    int conf;
    struct passwd* pw = getpwuid(getuid());

    /*send name for verification*/
    errNum(send(sock, pw->pw_name, strlen(pw->pw_name)+1, 0),
        "failed to send name", 4);

    /*recieve confirmation message*/
    printf("Waiting for response from %s\n", hostname);
    errNum(recv(sock, buffer, LEN, 0), "failed to recieve confirmation", 4);

    /*check confirmation message*/
    conf = (strcmp(buffer, GOMESSAGE) == 0);

    free(buffer);
    return conf;
}

/*
recvConfirmation recieves a name sent
via connected socket, checks the host
of that name, then asks the user if
they wish to open communications with
that user.
*/
int recvConfirmation(int sock, int autoConf, struct in_addr addr){
    const char* buffer = scalloc(LEN, sizeof(char), 
        "failed to intialize confirmation buffer", 5);
    char* message;
    int accept = 1;
    struct hostent* host = gethostbyaddr(&addr,sizeof(addr), AF_INET);

    /*recieve name from client*/
    errNum(recv(sock, buffer, LEN, 0),"failed to recieve name from client", 5);

    /*formulate full name + hostname*/
    message = tricat(buffer, MID, host->h_name);

    /*get user verification for confirmation*/
    if(!autoConf){
        printf("Mytalk request from %s. Accept? (y/n)? ", message);
        read_from_input(buffer, LEN);
        printf("\n");
    }

    /*verify user input*/
    if(strcmp(buffer, "yes") == 0 || strcmp(buffer, "y") == 0 ||
       strcmp(buffer, "yes\n") == 0 || strcmp(buffer, "y\n") == 0 || autoConf){

        /*send confirmation message*/
        errNum(send(sock, GOMESSAGE, 3,0), "failed to send GOMESSAGE", 5);
    }
    else{

        /*send negative message*/
        errNum(send(sock, NOMESSAGE, 3,0), "failed to send NOMESSAGE", 5);
        accept = 0;
    }

    free(message);
    free(buffer);
    return accept;
}