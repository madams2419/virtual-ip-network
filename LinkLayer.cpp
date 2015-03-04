#include <vector>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include "LinkLayer.h"
#include "constants.h"

#define MAX_MSG_LENGTH (512)
#define MAX_BACK_LOG (5)
#define SEND_LENGTH (MAX_MSG_LENGTH*3)

using namespace std;

LinkLayer::LinkLayer(phy_info localPhy, vector<itf_info> itfs) {
	this->localPhy = localPhy;
	this->itfs = itfs;

	localAI = new struct addrinfo;
	rcvSocket = createSocket(localPhy, localAI, true);
}

char* LinkLayer::getInterfaceAddr(int itf) {
	return itfs[itf].locAddr;
}

/**
 * Sends dataLen bytes of data over the interface specified by itfNum
 */
int LinkLayer::send(char* data, int dataLen, int itfNum) {
	int sendSocket, bytesSent;
	struct addrinfo *aiDest = new addrinfo;

	sendSocket = createSocket(itfs[itfNum].rmtPhy, aiDest, false);

	if ((bytesSent = sendto(sendSocket, data, dataLen, 0, aiDest->ai_addr, aiDest->ai_addrlen)) == -1) {
		perror("Send error:");
		return -1;
	}

	freeaddrinfo(aiDest);

	printf("Sent %d bytes over interface %d\n", bytesSent, itfNum);
	close(sendSocket);

	return bytesSent;
}

int LinkLayer::listen(char* buf, int bufLen) {
	int bytesRcvd;
	if ((bytesRcvd = recvfrom(rcvSocket, buf, bufLen-1 , 0, NULL, NULL)) == -1) {
		perror("Receive error:");
		return -1;
	}
	return bytesRcvd;
}

/**
 * Creates UDP socket and populates &aiRet with socket address info
 */
int LinkLayer::createSocket(phy_info phyInfo, struct addrinfo* aiRet, bool bindSock) {
	int sockfd, gai_ret;
	struct addrinfo aiHints, *aiList, *ai;

	// zero out address info hints structure
	memset(&aiHints, 0, sizeof aiHints);

	// populate address info hints
	aiHints.ai_family = AF_INET; // IPv4
	aiHints.ai_socktype = SOCK_DGRAM; // UDP

	if ((gai_ret = getaddrinfo(phyInfo.ipAddr, phyInfo.port, &aiHints, &aiList)) != 0) {
		perror("Get address info error:");
		return -1;
	}

	// loop through getaddrinfo() results. connect (and bind if flagged) to first one possible
	for(ai = aiList; ai != NULL; ai = ai->ai_next) {
		if ((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
			perror("Socket creation error:");
			continue;
		}

		if (bindSock) {
			if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) == -1) {
				close(sockfd);
				perror("Socking binding error:");
				continue;
			}
		}

		break;
	}

	if (ai == NULL) {
		printf("No valid address info structure found.\n");
		return -1;
	}

	// copy address info data
	*aiRet = *ai;

	// free address info list
	freeaddrinfo(aiList);

	return sockfd;
}

/* This main just for testing */
/*
int main(int argc, char ** argv) {
	int recv_len;
	char msg[512], reply[512];

	phy_info locPhy, rmtPhy;
	itf_info itf;
	vector<itf_info> itfs;

	locPhy.ipAddr = "127.0.0.1";
	rmtPhy.ipAddr = "127.0.0.1";

	locPhy.port = argv[2];
	rmtPhy.port = argv[3];

	itf.locAddr = "192.168.1.1";
	itf.rmtAddr = "192.168.1.1";
	itf.rmtPhy = rmtPhy;

	itfs.push_back(itf);

	LinkLayer* ll = new LinkLayer(locPhy, itfs);

	if (argv[1][0] == 'l') { // node is listener

		while (1) {
			perror("Listening for packets:\n");
			recv_len = ll->listen(reply, 512);
			if (recv_len < 0) {
				perror("Recv error:");
				return 1;
			}
			reply[recv_len] = 0;
			printf("Receive message:\n%s\n", reply);
			memset(reply, 0, sizeof(reply));
		}

	} else if (argv[1][0] == 's') { // node is sender

		while (1) {
			fflush(stdin);
			printf("Enter message: \n");
			gets(msg);
			if (ll->send(msg, 512, 0) < 0) {
				perror("Send error:");
				return 1;
			}
		}

	} else { // invalid argument
		printf("Invalid argument.");
	}
}
*/
