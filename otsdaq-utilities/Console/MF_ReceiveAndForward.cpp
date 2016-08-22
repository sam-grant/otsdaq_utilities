// ots_udp_hw_emulator.cpp
//    by rrivera at fnal dot gov
//	  created Feb 2016
//
// This is a simple emulator of a "data gen" front-end (hardware) interface
// using the otsdaq UDP protocol.
//
//compile with:
//g++ ots_udp_hw_emulator.cpp -o hw.o
//
//if developing, consider appending -D_GLIBCXX_DEBUG to get more 
//descriptive error messages
//
//run with:
//./hw.o
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;


#define MAXBUFLEN 5000

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int makeSocket(const char * ip, int port, struct addrinfo*& p)
{
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	char portStr[10];
	sprintf(portStr,"%d",port);
	if ((rv = getaddrinfo(ip, portStr, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("sw: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "sw: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	return sockfd;
}

int main(int argc, char** argv)
{
	std::string myPort_("3000");
	std::string myFwdPort_("3001");
	if(argc >= 2)
		myPort_ = argv[1];
	if(argc >= 3)
		myFwdPort_ = argv[2];

	int myFwdPort;
	sscanf(myFwdPort_.c_str(),"%d",&myFwdPort);

	int sockfd;
    int sendSockfd=0;

    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buff[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL,
    		myPort_.c_str(),
			&hints,
			&servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    //////////////////////////////////////////////////////////////////////
    ////////////// ready to go //////////////
    //////////////////////////////////////////////////////////////////////




    //hardware "registers"
	uint64_t 	data_gen_cnt = 0;
	uint64_t 	data_gen_rate = 1<<16;
	uint8_t		dataEnabled = 0;

    const unsigned int RX_ADDR_OFFSET = 2;
    const unsigned int RX_DATA_OFFSET = 10;
    const unsigned int TX_DATA_OFFSET = 2;

    unsigned int packetSz;

    //for timeout/select
    struct timeval tv;
	fd_set readfds, masterfds;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	FD_ZERO(&masterfds);
	FD_SET(sockfd, &masterfds);

	time_t count = 0;

	int mf_p,mf_i; //for extracting message
	const int MF_POS_OF_MSG = 11;

    //this should ip/port of Console xdaq app Receiver port
	sendSockfd = makeSocket("127.0.0.1", myFwdPort, p);

    while(1)
    {
    	readfds = masterfds; //copy to reset timeout select
		select(sockfd+1, &readfds, NULL, NULL, &tv);

	    if (FD_ISSET(sockfd, &readfds))
	    {
	    	//packet received
	    	//cout << "hw: Line " << __LINE__ << ":::" << "Packet Received!" << endl;

			addr_len = sizeof their_addr;
			if ((numbytes = recvfrom(sockfd, buff, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}

			//			printf("hw: got packet from %s\n",
			//				inet_ntop(their_addr.ss_family,
			//					get_in_addr((struct sockaddr *)&their_addr),
			//					s, sizeof s));
			//			printf("hw: packet is %d bytes long\n", numbytes);
			//			printf("packet contents: ");
			//
			//			for(int i=0;i<numbytes;++i)
			//			{
			//				if((i-RX_ADDR_OFFSET)%8==0) printf("\n");
			//				printf("%2.2X", (unsigned char)buff[i]);
			//			}
			//			printf("\n");






			//print message without decoration
			//find position of message and save to p
							//by jumping to the correct '|' marker
			buff[numbytes] = '\0'; //make sure it is null terminated

			//DEBUG -- for indentifying strange MessageFacility bug with clipped messages
			//std::cout << "+" << ((int)strlen(buff)==numbytes?1:0) << " " << buff << std::endl;
			//			if((int)strlen(buff)!=numbytes)
			//			{
			//				for(int iii=strlen(buff)-3;iii<numbytes;++iii)
			//					std::cout << (int)buff[iii] << "-" << (char)buff[iii] << " ";
			//				std::cout << numbytes << " " << strlen(buff) << std::endl;
			//				std::cout << std::endl;
			//			}

			//count markers to find message
			for(mf_p=0,mf_i=0;mf_i<numbytes && mf_p<MF_POS_OF_MSG;++mf_i)
				if(buff[mf_i] == '|') ++mf_p; //count markers

			if(mf_i<numbytes) //if valid find, show message
				std::cout << &buff[mf_i-1] << std::endl; // show leading '|'







			//forward packet onto sendSockfd

			if ((numbytes = sendto(sendSockfd, buff, numbytes, 0, p->ai_addr, p->ai_addrlen)) == -1) {
				perror("hw: sendto");
				exit(1);
			}
			//printf("hw: sent %d bytes on\n", numbytes);

	    }
    }

    close(sockfd);
    close(sendSockfd);

    return 0;
}

