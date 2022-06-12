/*
** talker.c -- a datagram "client" demo
* taken from: https://beej.us/guide/bgnet/html/index-wide.html#datagram
* modified for c++11 and windows support
*/

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

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

typedef int SOCKET;
#endif

#define SERVERPORT "45001"	// the port users will be connecting to
uint32_t data_size = 2052;

int main(int argc, char *argv[])
{
	uint32_t idx;
	SOCKET sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	uint32_t count = 0;

	std::vector<uint32_t> tx_data(data_size);

	if (argc < 2) 
	{
		fprintf(stderr,"usage: talker hostname\n");
		exit(1);
	}

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

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) 
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) 
    {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) 
    {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	// generate some data to send
	for(idx=0; idx< data_size; ++idx)
	{
		tx_data[idx] = idx;
	}

	while (1)
	{
		tx_data[0] = count++;

		numbytes = sendto(sockfd, (char*)tx_data.data(), tx_data.size() * sizeof(uint32_t), 0, p->ai_addr, p->ai_addrlen);
		if (numbytes == -1)
		{
			perror("talker: sendto");
		}
		else
			printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	}


	
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	result = shutdown(sockfd, SD_SEND);
	if (result == SOCKET_ERROR) 
	{
		//error_msg = "Shutdown failed with error: (" + std::to_string(result) + " : " + std::to_string(WSAGetLastError()) + ")";
		closesocket(sockfd);
		WSACleanup();
	}

#else
	close(sockfd);

#endif

	return 0;
}
