#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>


int main(int argc, char const *argv[])
{
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return -1;
	}
	int tcp_fd, port = atoi(argv[2]);
	struct sockaddr_in addr_server;
	const char *server_ip = argv[1];
	
	//Adding address to addr_server
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(port);
	
	//Creating client socket.
	tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_fd < 0){
		printf("[*] Error Creating socket\n");
		return -1;
	}
	printf("[*] Client's socket is created successfully\n");
	
	if(inet_pton(AF_INET, server_ip, &addr_server.sin_addr) <= 0){
		printf("[*] Invalid Server Address\n");
		return -1;
	}
	
	//Trying to connect to the server.
	if(connect(tcp_fd, (struct sockaddr *) &addr_server, sizeof(addr_server))){
		printf("[*] Connection to Server Failed.\n");
		return -1;
	}
	printf("[*] Connection to server established.\n");
	
	//Sending Type 1 Message
	char *type_1_msg = "1 26 Send me a UDP port.\n";
	write(tcp_fd, type_1_msg, strlen(type_1_msg));
	
	//Reading type 2 Message UDP port number from server.
	char buff[1024];
	read(tcp_fd, buff, sizeof(buff));
	close(tcp_fd);
	int udp_server_port, t, t1;
	sscanf(buff, "%d %d %d", &t, &t1, &udp_server_port);
	printf("[*] Server:\t%s\n", buff);
	printf("[*] Port #%d recieved from server\n", udp_server_port);
	
	//Establish UDP connection to the server.
	int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_socket < 0){
		printf("Cannot create UDP socket.\n");
		return -1;
	}
	
	// Filling in server details
	struct sockaddr_in addr_server_udp;
	bzero(&addr_server_udp, sizeof(addr_server_udp));
	addr_server_udp.sin_family = AF_INET;
	addr_server_udp.sin_port = htons(udp_server_port);
	addr_server_udp.sin_addr.s_addr = inet_addr(server_ip);
	
	if(connect(udp_socket, (struct sockaddr *) &addr_server_udp, sizeof(addr_server_udp)) < 0){
		printf("[*] UPD connection failed.\n");
		return -1;
	}
	printf("[*] UDP connection to the server established.\n");
	
	// Sending Type 3 message to the server
	sendto(udp_socket, "3 40 Hello from Client\n", strlen("3 40 Hello from Client\n"), MSG_CONFIRM, (struct sockaddr *)NULL, sizeof(addr_server));
	
	//Recieving Type 4 message from server.
	recvfrom(udp_socket, buff, sizeof(buff), 0, (struct sockaddr *)NULL, NULL);
	
	printf("[*] Server:\t%s\n\n", buff);
	close(udp_socket);
}
