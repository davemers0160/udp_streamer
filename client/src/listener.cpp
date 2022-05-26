/*
** listener.c -- a datagram sockets "server" demo
* taken from: https://beej.us/guide/bgnet/html/index-wide.html#datagram
* modified for c++11 and windows support
*/

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#pragma comment(lib, "iphlpapi.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#else
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typdef SOCKET int
#endif

#define MYPORT "45001"	// the port users will be connecting to
const uint32_t data_size = 2052;
#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// ----------------------------------------------------------------------------
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
uint32_t close_connection(SOCKET& s, std::string& error_msg)
{
	int32_t result = shutdown(s, SD_SEND);
	if (result == SOCKET_ERROR) {
		error_msg = "Shutdown failed with error: (" + std::to_string(result) + " : " + std::to_string(WSAGetLastError()) + ")";
		closesocket(s);
		WSACleanup();
		return 10;
	}

	return 0;

}   // end of close_connection
#endif

int main(void)
{
	uint32_t idx;
	SOCKET sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	std::string error_msg = "";

	std::vector<uint32_t> tx_data(65536);

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	const uint16_t dllVersion = MAKEWORD(2, 2);
	WSADATA wsaData;

	// Initialize Winsock
	int result = WSAStartup(dllVersion, &wsaData);
	if (result != 0) {
		//error_msg = "Winsock startup failed. Error: " + std::to_string(result);
		return 1;
	}
#endif

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
			close_connection(sockfd, error_msg);
#else
			close(sockfd);
#endif
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
	int buffsize = 50000; 
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	numbytes = recvfrom(sockfd, (char*)tx_data.data(), data_size*4 - 1, 0, (struct sockaddr*)&their_addr, &addr_len);
	if (numbytes == SOCKET_ERROR)
	{
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
		result = WSAGetLastError();
		std::cout << "error: " << result << std::endl;
#endif
		perror("recvfrom");
		//exit(1);
	}

	printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	//buf[numbytes] = '\0';
	for (idx = 0; idx < data_size; ++idx)
	{
		std::cout << tx_data[idx] << std::endl;
	}
	//printf("listener: packet contains \"%s\"\n", buf);

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	close_connection(sockfd, error_msg);
#else
	close(sockfd);
#endif

	return 0;
}