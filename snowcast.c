#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>

int server(uint16_t port);
int client(const char * addr, uint16_t port);
void *handleConnection(void *sockfd_ptr);

#define MAX_MSG_LENGTH (512)
#define MAX_BACK_LOG (5)
#define SEND_LENGTH (MAX_MSG_LENGTH*3)

int main(int argc, char ** argv)
{
	if (argc < 3) {
		printf("Command should be: myprog s <port> or myprog c <port> <address>\n");
		return 1;
	}
	int port = atoi(argv[2]);
	if (port < 1024 || port > 65535) {
		printf("Port number should be equal to or larger than 1024 and smaller than 65535\n");
		return 1;
	}
	if (argv[1][0] == 'c') {
		if (argv[3]==NULL){
			printf("NO IP address is given\n");
			return 1;
		}
		return client(argv[3], port);
	} else if (argv[1][0] == 's') {
		return server(port);
	} else {
		printf("unknown command type %s\nCommand should be: myprog s <port> or myprog c <port> <address>", argv[1]);
		return 1;
	}
	return 0;
}

int client(const char * addr, uint16_t port)
{
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH*3];

	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) {
		perror("Create socket error:");
		return 1;
	}

	printf("Socket created\n");
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connect error:");
		return 1;
	}

	printf("Connected to server %s:%d\n", addr, port);

	int recv_len = 0;
	while (1) {
		fflush(stdin);
		printf("Enter message: \n");
		gets(msg);
		if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
			perror("Send error:");
			return 1;
		}
		recv_len = read(sock, reply, MAX_MSG_LENGTH*3);
		if (recv_len < 0) {
			perror("Recv error:");
			return 1;
		}
		reply[recv_len] = 0;
		printf("Server reply:\n%s\n", reply);
		memset(reply, 0, sizeof(reply));
	}
	close(sock);
	return 0;
}

int server(uint16_t port)
{
	int sockfd_ls; // listen socket descriptor
	long sockfd_ac; // accept socket descriptor
	char port_as_str[2]; // connection port as a string
	struct addrinfo ai_hints; // hints address info
	struct addrinfo *ai_list; // linked list of address info structs returned by get address info
	struct addrinfo *ai; // valid address info
	struct sockaddr_storage *client_addr; // client address
	socklen_t client_addr_size = sizeof(client_addr); // size of client address
	pthread_t thread_pool[MAX_BACK_LOG];

	// convert int port to str
	sprintf(port_as_str, "%u", port);

	// fill hints address info
	memset(&ai_hints, 0, sizeof(ai_hints)); // zero out struct
	ai_hints.ai_family = AF_INET; // IPv4
	ai_hints.ai_socktype = SOCK_STREAM; // TCP
	ai_hints.ai_flags = AI_PASSIVE; // tell gai to assign local host address to socket structs

	// get address info
	if (getaddrinfo(NULL, port_as_str, &ai_hints, &ai_list) != 0) {
		perror("Get address info error:");
		return 1;
	}

	// loop through ai linked list and open socket on first valid addrinfo struct
	for (ai = ai_list; ai != NULL; ai = ai->ai_next) {

		// try to open a socket
		if ((sockfd_ls = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) != -1) {

			// try to bink socket to port
			if (bind(sockfd_ls, ai->ai_addr, ai->ai_addrlen) == 0) {

				// socket opened and bound,
				break;

			} else perror("Socket binding error:");

		} else perror("Socket open error:");
	}

	// return error if no valid addrinfos found
	if (ai == NULL) {
		printf("No valid address info structure found.\n");
		return 1;
	}

	// listen for incoming connections
	if (listen(sockfd_ls, MAX_BACK_LOG) != 0) {
		perror("Listen error:");
		return 1;
	}

	// loop for client connections
	while (1) {
		// accept client connection
		sockfd_ac = accept(sockfd_ls, (struct sockaddr*) client_addr, &client_addr_size);

		// create worker thread
		pthread_t worker_thread;

		// dispatch thread to handle the connection
		int rc = pthread_create(&worker_thread, NULL, handleConnection, (void *)sockfd_ac);
		if (rc) {
			perror("Threading error:");
			return -1;
		}
	}

	// close sockets
	close(sockfd_ls);
	close(sockfd_ac);

	// free addrinfo linked list
	freeaddrinfo(ai_list);

	// exit threads
	pthread_exit(NULL);

	return 0;
}

void *handleConnection(void *sockfd_ptr) {
	int sockfd = (int) sockfd_ptr;
	// loop for client messages
	while (1) {
		char rcv_buf[MAX_MSG_LENGTH + 1]; // receive message buffer
		char send_buf[SEND_LENGTH + 1]; // send message buffer
		ssize_t rcv_len; // receive message length
		ssize_t send_len; // send message length

		// add null characer at end of rcv buffer in case message string length is >= 512
		// in this case, the null terminator will not be sent
		rcv_buf[MAX_MSG_LENGTH] = 0;

		// receive message from client
		rcv_len = recv(sockfd, (void*) rcv_buf, (size_t) MAX_MSG_LENGTH, 0);

		if (rcv_len <= 0) { // client has closed the connection
			break;
		} else if (rcv_len == -1) { // receive error
			perror("Receive error:");
			continue;
		}

		// build response message
		strcpy(send_buf, rcv_buf);
		strcat(send_buf, rcv_buf);
		strcat(send_buf, rcv_buf);

		// respond to client
		if ((send_len = send(sockfd, (const void*) send_buf, SEND_LENGTH, 0)) == -1) {
			perror("Send error:");
			break;
		}
	}
	pthread_exit(NULL);
}
