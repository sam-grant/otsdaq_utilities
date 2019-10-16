// MF_ReceiveAndForward.cpp
//    by rrivera at fnal dot gov
//	  created Feb 2016
//
// 	This is a simple UDP receive and forward program
//		for MessageFacility packets.
//
//	It echos packets received and only appends '|' as decoration.
//
//
//
//
// compile with:
// g++ MF_ReceiveAndForward.cpp -o MF_ReceiveAndForward.o
//
// if developing, consider appending -D_GLIBCXX_DEBUG to get more
// descriptive error messages
//
// run with:
//./MF_ReceiveAndForward.o <optional port file name>
//
//
//	Port Config File Format:
//		RECEIVING_PORT 	<port number>
//		FORWARDING_PORT <port number>
//
//

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define MAXBUFLEN 5000

#define __MF_SUBJECT__ "mfReceiveAndForward"
#define Q(X) #X
#define QUOTE(X) Q(X)

// take filename only after srcs/ (this gives by repo name)
#define __SHORTFILE__ \
	(strstr(&__FILE__[0], "/srcs/") ? strstr(&__FILE__[0], "/srcs/") + 6 : __FILE__)

// take only file name
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define __E__ std::endl

#define __COUT_HDR_F__ __SHORTFILE__ << "\t"
#define __COUT_HDR_L__ "[" << std::dec << __LINE__ << "]\t"
#define __COUT_HDR_P__ __PRETTY_FUNCTION__ << "\t"
#define __COUT_HDR_FL__ __SHORTFILE__ << " " << __COUT_HDR_L__
#define __COUT_HDR_FP__ __SHORTFILE__ << " : " << __COUT_HDR_P__
#define __COUT_HDR__ __COUT_HDR_FL__

#define __COUT_TYPE__(X) std::cout << QUOTE(X) << ":" << __MF_SUBJECT__ << ":"

#define __COUT_ERR__ __COUT_TYPE__(LogError) << __COUT_HDR__
#define __COUT_WARN__ __COUT_TYPE__(LogWarning) << __COUT_HDR__
#define __COUT_INFO__ __COUT_TYPE__(LogInfo) << __COUT_HDR__
#define __COUT__ __COUT_TYPE__(LogDebug) << __COUT_HDR__
#define __COUTV__(X) __COUT__ << QUOTE(X) << " = " << X << __E__

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr* sa)
{
	if(sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int makeSocket(const char* ip, int port, struct addrinfo*& p)
{
	int                     sockfd;
	struct addrinfo         hints, *servinfo;
	int                     rv;
	int                     numbytes;
	struct sockaddr_storage their_addr;
	socklen_t               addr_len;
	char                    s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	char portStr[10];
	sprintf(portStr, "%d", port);
	if((rv = getaddrinfo(ip, portStr, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("sw: socket");
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "sw: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	return sockfd;
}

int main(int argc, char** argv)
{
	__COUT__ << "Starting...\n\n" << __E__;

	std::string myPort_("3000");        // set default
	std::string myFwdPort_("3001");     // set default
	std::string myFwdIP_("127.0.0.1");  // set default
	if(argc >= 2)
	{
		__COUT__ << "port parameter file:" << argv[1] << "\n\n" << __E__;
		FILE* fp = fopen(argv[1], "r");
		if(fp)
		{
			char tmp[100];
			char tmpParamStr[100];
			fgets(tmp, 100, fp);
			sscanf(tmp, "%*s %s", tmpParamStr);
			myPort_ = tmpParamStr;
			fgets(tmp, 100, fp);
			sscanf(tmp, "%*s %s", tmpParamStr);
			myFwdPort_ = tmpParamStr;
			fgets(tmp, 100, fp);
			sscanf(tmp, "%*s %s", tmpParamStr);
			myFwdIP_ = tmpParamStr;
			fclose(fp);
		}
		else  // else use defaults
			__COUT__ << "port parameter file failed to open: " << argv[1] << "\n\n"
			         << __E__;
	}
	__COUT__ << "Forwarding from: " << myPort_ << " to: " << myFwdIP_ << ":" << myFwdPort_
	         << "\n\n"
	         << __E__;

	int myFwdPort;
	sscanf(myFwdPort_.c_str(), "%d", &myFwdPort);

	int sockfd;
	int sendSockfd = 0;

	struct addrinfo         hints, *servinfo, *p;
	int                     rv;
	int                     numbytes;
	struct sockaddr_storage their_addr;
	char                    buff[MAXBUFLEN];
	socklen_t               addr_len;
	char                    s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC;  // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags    = AI_PASSIVE;  // use my IP

	if((rv = getaddrinfo(NULL, myPort_.c_str(), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			__COUT__ << "listener: socket...\n\n" << __E__;
			perror("listener: socket");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			__COUT__ << "listener: bind.\n\n" << __E__;
			perror("listener: bind");
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		__COUT__ << "listener: failed to bind socket...\n\n" << __E__;
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	//////////////////////////////////////////////////////////////////////
	////////////// ready to go //////////////
	//////////////////////////////////////////////////////////////////////

	// hardware "registers"
	uint64_t data_gen_cnt  = 0;
	uint64_t data_gen_rate = 1 << 16;
	uint8_t  dataEnabled   = 0;

	const unsigned int RX_ADDR_OFFSET = 2;
	const unsigned int RX_DATA_OFFSET = 10;
	const unsigned int TX_DATA_OFFSET = 2;

	unsigned int packetSz;
	unsigned int pingCounter = 0;

	// for timeout/select
	struct timeval tv;
	fd_set         readfds, masterfds;
	tv.tv_sec  = 0;
	tv.tv_usec = 500000;
	FD_ZERO(&masterfds);
	FD_SET(sockfd, &masterfds);

	time_t count = 0;

	int       mf_p, mf_i, mf_j;  // for extracting message
	const int MF_POS_OF_TYPE = 5;
	const int MF_POS_OF_MSG  = 11;
	bool      firstPartPresent;

	// this should ip/port of Console xdaq app Receiver port
	sendSockfd = makeSocket(myFwdIP_.c_str(), myFwdPort, p);

	while(1)
	{
		readfds = masterfds;  // copy to reset timeout select
		select(sockfd + 1, &readfds, NULL, NULL, &tv);

		if(FD_ISSET(sockfd, &readfds))
		{
			pingCounter = 0; //reset ping counter

			// packet received
			// cout << "hw: Line " << __LINE__ << ":::" << "Packet Received!" << endl;

			addr_len = sizeof their_addr;
			if((numbytes = recvfrom(sockfd,
			                        buff,
			                        MAXBUFLEN - 1,
			                        0,
			                        (struct sockaddr*)&their_addr,
			                        &addr_len)) == -1)
			{
				__COUT__ << "error: recvfrom...\n\n" << __E__;
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

			// print message without decoration
			// find position of message and save to p
			// by jumping to the correct '|' marker
			buff[numbytes] = '\0';  // make sure it is null terminated

			// DEBUG -- for identifying strange MessageFacility bug with clipped messages
			// std::cout << "+" << ((int)strlen(buff)==numbytes?1:0) << " " << buff <<
			// __E__; 			if((int)strlen(buff)!=numbytes)
			//			{
			//				for(int iii=strlen(buff)-3;iii<numbytes;++iii)
			//					std::cout << (int)buff[iii] << "-" << (char)buff[iii] << "
			//"; 				std::cout << numbytes << " " << strlen(buff) << __E__;
			//				std::cout << __E__;
			//			}

			// count markers to find message

			// std::cout << "|||" << buff << __E__; // show all
			// e.g. UDPMFMESSAGE7370|01-Jul-2019 11:12:44
			// CDT|3|correlator2.fnal.gov|131.225.52.45|Info|_TCPConnect|xdaq.exe|7370|Booted|DAQ|TCPConnect.cc|241|Resolving
			// ip correlator2.fnal.gov

			for(mf_p = 0, mf_i = 0; mf_i < numbytes && mf_p < MF_POS_OF_TYPE; ++mf_i)
				if(buff[mf_i] == '|')
					++mf_p;  // count markers

			for(mf_j = mf_i; mf_j < numbytes && mf_p < MF_POS_OF_TYPE + 1; ++mf_j)
				if(buff[mf_j] == '|')
					++mf_p;  // count markers

			// print first part (message type)
			if(mf_i < mf_j && mf_j < numbytes)
			{
				char sav       = buff[mf_j - 1];
				buff[mf_j - 1] = '\0';
				std::cout << &buff[mf_i - 1];
				buff[mf_j - 1] = sav;

				// tab for all types but Warning
				if(strcmp(&buff[mf_i - 1], "|Warning") != 0)
					std::cout << "\t";

				firstPartPresent = true;
			}
			else
				firstPartPresent = false;

			for(mf_i = mf_j; mf_i < numbytes && mf_p < MF_POS_OF_MSG; ++mf_i)
				if(buff[mf_i] == '|')
					++mf_p;  // count markers

			// print second part
			if(mf_i < numbytes)                         // if valid find, show message
				std::cout << &buff[mf_i - 1] << __E__;  // show msg after '|'
			else if(firstPartPresent)
				std::cout << __E__;

			// forward packet onto sendSockfd

			if((numbytes = sendto(
			        sendSockfd, buff, numbytes, 0, p->ai_addr, p->ai_addrlen)) == -1)
			{
				__COUT__ << "error: sendto...\n\n" << __E__;
				perror("hw: sendto");
				exit(1);
			}
			// printf("hw: sent %d bytes on\n", numbytes);
		}
		else
		{
			sleep(1);  // one second
			if(++pingCounter > 2*60) //two minutes
			{
				//send 1-byte "ping" to keep socket alive
				if((numbytes = sendto(
						sendSockfd, buff, 1, 0, p->ai_addr, p->ai_addrlen)) == -1)
				{
					__COUT__ << "error: ping sendto...\n\n" << __E__;
					perror("hw: ping sendto");
					exit(1);
				}
				pingCounter = 0;
			}
		}
	}

	close(sockfd);
	close(sendSockfd);

	__COUT__ << "Exited.\n\n" << __E__;

	return 0;
}
