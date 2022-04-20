#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include<arpa/inet.h>

int siteName(char* a, char* url);
struct sockaddr_in siteIP( char* url);
void message(int urlSize, char* msg, char* request);

int main(int argc, char *argv[]){
	//to hold the website url
	char url[100];
	//22 is ucalgary url size, this is the default
	int urlSize=22;
	//for the GET request
	char request[5000]="GET ";
	char req[5000];
	//to recive information 
	int recvStatus;

	//for the fork
	pid_t pid;
	//sockets for communicaion, 1 is to establish connection, other is to recieve input from browser, 
	//last is to send and recieve data from the website
	int socket_desc, client_sock, connection_fd;
	//sockaddr for website and local 
	struct sockaddr_in server, netCom;
	//message from the client
	char client_message[5000];

	//for reading from the website
	int bytes=0;
	char buffer[50001] = {0};
	
	
	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket\n");
	}
	puts("Socket created\n");
	
	//Prepare the sockaddr_in structure
	memset(&server,0,sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(11111);
	

	//Bind
    int bindStatus = bind(socket_desc, (struct sockaddr *)&server, sizeof(server));
	if( bindStatus == -1){
		//print the error message
		perror("Binding failed");
		return 1;
	}
	printf("Binding done.\n");
	
	//Listen
	listen(socket_desc, 3);
	
	//Accept and incoming connection
	printf("Waiting for clients...\n");
	
	
	while(1){
		//accept connection from an incoming client
		client_sock = accept(socket_desc, NULL, NULL);
		if (client_sock < 0){
			perror("Connection failed");
			return 1;
		}
		printf("Connection accepted\n");
		

		//creating the fork
		pid=fork();

		//checking if the fork has failed
		if(pid<0){
			printf("fork call failed");
			exit(1);
		//if it succeds
		}else if(pid==0){
			//connection is made so this is no longer needed
			close(socket_desc);

			//recieving message from client
			recvStatus = recv(client_sock, client_message, 5000, 0);
			if (recvStatus==-1){
				printf("Error in receiving!\n");
				//break;
			}

			//client message
			printf("Client says: %s\n", client_message);


			//extracting the URL from the client and the size of it
			urlSize=siteName(client_message,url);
			printf("\nclient URL is: %s, and url size is: %d\n",url,urlSize);

			
			//creating the socket that will connect to the site
			connection_fd = socket(AF_INET,SOCK_STREAM,0);
			if(connection_fd == -1) { 
				perror("Failed to create socket"); 
				return 1;
			}
			printf("net socket created\n");
			//creating the sockaddr structure 
			netCom=siteIP(url);

			//connecting
			if(connect(connection_fd,(struct sockaddr*)&netCom, sizeof(struct sockaddr_in))==-1)
				printf("error in connecting");
			printf("connect to site\n");
			
			//getting the GET request
			message(urlSize,client_message,req);
			strcat(request,req);
			printf("message is%s\n",request);

			//replacing all gif files
			if(strstr(request,"jpg"))){
				//randomly selecting between clown 1 or 2
				if(rand()%2==0){
					send(connection_fd, "GET /~carey/CPSC441/ass1/clown1.png HTTP/1.0\r\n\r\n", 
					sizeof("GET /~carey/CPSC441/ass1/clown1.png HTTP/1.0\r\n\r\n"), 0);
				}else{
					send(connection_fd, "GET /~carey/CPSC441/ass1/clown2.png HTTP/1.0\r\n\r\n", 
					sizeof("GET /~carey/CPSC441/ass1/clown2.png HTTP/1.0\r\n\r\n"), 0);
				}
			}else{
				//sending the GET request
				if(send(connection_fd,request,sizeof(request),0)==-1)
				printf("error in sending\n");
			}

			//resetting the request for the next time
			memset(request,'\0',sizeof(request));
			request[0]='G';
			request[1]='E';
			request[2]='T';
			request[3]=' ';

			//getting the information from the web
			do
			{
				memset(buffer, 0, 5001);
				bytes = recv(connection_fd, buffer, 5000, 0);
				printf("buffer says %s",buffer);

				//replacing happy with silly
				char* happy=strstr(buffer,"Happy");
				while(happy!=NULL){
					*happy='s';
					*(happy+1)='i';
					*(happy+2)='l';
					*(happy+3)='l';
					*(happy+4)='y';
					happy=strstr(buffer,"Happy");
				}

				send(client_sock, buffer, bytes, 0);
			} while (bytes > 0);
			//closing the sockets that were used
			close(client_sock);
			close(connection_fd);
			//ending with a now line in case the last print statement didn't (learned this the hard way)
			printf("\n");
			//exiting to prevent infinite forks (learned this the hard way too)
			exit(0);
		}else{
			/* the parent process is the one doing this part */
            fprintf(stderr, "Server created child process %d to handle that client\n", pid);
            fprintf(stderr, "Main server process going back to listening for new clients now...\n\n");
		}

	}
	
	return 0;
}

int siteName(char* a, char* url){
	int i=0;
	for( i=11;a[i]!='/';i++){
		url[i-11]=a[i];
		
	}
	return i-11;
}
struct sockaddr_in siteIP( char* url){
	struct hostent *addr;
		addr = gethostbyname(url);
		struct sockaddr_in connection_sa;
		memset(&connection_sa,0,sizeof(struct sockaddr_in));
		connection_sa.sin_family = AF_INET;
		connection_sa.sin_port = htons(80);
		bcopy(addr->h_addr_list[0],&connection_sa.sin_addr.s_addr,addr->h_length);
	return connection_sa;

}

void message(int urlSize, char* msg, char* request){
	int i=0;
	int startIndex=urlSize+11;
	for(i=startIndex;msg[i]!=' ';i++){
		request[i-startIndex]=msg[i];
	}
	i-=startIndex;
	request[i]=' ';
	request[i+1]='H';
	request[i+2]='T';
	request[i+3]='T';
	request[i+4]='P';
	request[i+5]='/';
	request[i+6]='1';
	request[i+7]='.';
	request[i+8]='0';
	request[i+9]='\r';
	request[i+10]='\n';
	request[i+11]='\r';
	request[i+12]='\n';
	
}
