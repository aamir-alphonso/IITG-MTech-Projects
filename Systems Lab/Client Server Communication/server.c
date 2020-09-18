#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>

// This function starts trying incrementally for every port whichever is available.
int get_udp_port(int udp_socket, int port, struct sockaddr_in *addr_server, int size){
	addr_server->sin_family = AF_INET;
	addr_server->sin_addr.s_addr = htonl(INADDR_ANY);
	while(1){
		addr_server->sin_port = htons(port);
		if(bind(udp_socket, (const struct sockaddr *) addr_server, size) >=0){
			return port;
		}
		port++;
	}
	return -1;
}

void udp_connection(int udp_socket, struct sockaddr_in *addr_server){
	char buf[1024];
	struct sockaddr_in addr_client;
	int len = sizeof(struct sockaddr_in);
	bzero(&addr_client, sizeof(addr_client));
	printf("[*] UDP connection with client established.\n");
	
	//Receive type 3 message from client.
	int n = recvfrom(udp_socket, buf, sizeof(buf), 0, (struct sockaddr *)&addr_client, (socklen_t *) &len);
	buf[n] = '\0';
	printf("[*] Client(UDP):\t%s\n", buf);
	
	//Send type 4 response message from the client.
	sendto(udp_socket, "4 30 Your message has been successfully delivered.\n", strlen("4 30 Your message has been successfully delivered.\n"), 
							0, (struct sockaddr *)&addr_client, len);
	printf("\n\n");
}

int new_tcp_socket(int tcp_fd, struct sockaddr_in *tcp_sock_addr){
	int len_addr = sizeof(tcp_sock_addr);
	int new_tcp_fd = accept(tcp_fd, (struct sockaddr *) &tcp_sock_addr, (socklen_t *) &len_addr);
	int udp_socket, udp_port;
	struct sockaddr_in addr_server;
	if(new_tcp_fd < 0){
		printf("[*] Cannot create new socket for Client ...\n");
		return -1;
	}
	char buffer[2048];
	
	// Reading Type 1 message
	read(new_tcp_fd, buffer, 2048);
	printf("[*] Client:\t%s", buffer);
	
	//As soon as it recieves a message from a client it will fork into new process
	// And the old process will continue listening to other clients concurrently.
	if(fork()==0){
		//Obtaining new UPD port
		if((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			printf("[*] Socket creation failed for Client ...\n");
			return -1;
		}
		udp_port = get_udp_port(udp_socket, 10000, &addr_server, sizeof(addr_server));
		
		//Sending Type 2 message to Client, and closing the tcp conn.
		char type_2_msg[1024];
		sprintf(type_2_msg, "2 9 %d", udp_port);
		printf("[*] Sending port %d to client.\n", udp_port);
		write(new_tcp_fd, type_2_msg, sizeof(type_2_msg));
		close(new_tcp_fd);
		
		//Moving work over to udp
		udp_connection(udp_socket, &addr_server);
	}
	return 0;
}


int main(int argc, char const *argv[])
{
	if(argc != 2){
		printf("[*] Wrong Number of arguments\n");
		return -1;
	}
	int tcp_fd, port = atoi(argv[1]);
	struct sockaddr_in tcp_sock_addr;
	
	//Creating socket file descriptor
	if((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		printf("[*] Error creating socket\n");
		return -1;
	}
	
	// Adding information to socket address
	tcp_sock_addr.sin_family = AF_INET;
	tcp_sock_addr.sin_addr.s_addr = INADDR_ANY;
	tcp_sock_addr.sin_port = htons(port);
	
	// Binding socket file descriptor to respective port;
	if(bind(tcp_fd, (struct sockaddr *) &tcp_sock_addr, sizeof(tcp_sock_addr)) < 0){
		printf("[*] Binding the socket to port %d failed\n", port);
		return -1;
	}
	printf("[*] Binding to port %d done.\n", port);
	
	// Enabling listen of server with queue size 30
	if(listen(tcp_fd, 30) < 0){
		printf("[*] Listen failed\n");
		return -1;
	}
	printf("[*] Server now listening.\n");
	
	while(1){
		new_tcp_socket(tcp_fd, &tcp_sock_addr);
	}
}
