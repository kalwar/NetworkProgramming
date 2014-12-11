//This program is done by
//Student name:: Santosh Kumar Kalwar
//Student number:: 0331927
//Reference taken from 
//http://www.it.lut.fi/kurssit/07-08/CT30A5000/home-exam.html
//http://www.faqs.org/rfcs/rfc959.html
//Stevens, W.: TCP/IP Illustrated Volume 1, page 419
//http://beej.us/guide/bgnet/output/htmlsingle/bgnet.html

//Include Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

//FTP commands
#define GET 1
#define PUT 2
#define OPEN 3
#define CLOSE 4
#define LS 5
#define CD 6
#define QUIT 7
#define CDUP 8
#define PASV 9

//Function declarations
int ftpfun(int,int,int,struct sockaddr_in);
int parsfun(char*);
int openfun(int,struct sockaddr_in);
int getfun(int,int,int,char*,int,struct sockaddr_in);
int putfun(int,int,int,char*,int,struct sockaddr_in);
int closefun(int);
int listfun(int,int,int,char*,int,struct sockaddr_in);
int cdfun(int,char*);
int cdupfun(int);
int passivefun(int,struct sockaddr_in*);
int receive(int,char*);
int dsoket(int,int,int);
int pdsocket(int,int,struct sockaddr_in*);

//Main Program Starts here
int main(int argc, char** argv){
	int n,port,dport,sfd=-1,dsfd,l;
	struct sockaddr_in o,od;
	struct hostent *hp;
	
	//Check parameters
	if((argc!=7)||(strcmp("-p",argv[1]))||(strcmp("-P",argv[3]))||(strcmp("-h",argv[5]))){
		printf("Use: %s -p <FTP server port> -P <data transfer port> -h <ftp server address>\n",argv[0]);
		return -1;
	}

	//Check port
	port=atoi(argv[2]);
	if(port<1){
		printf("Incorrect FTP port\n");
		return -1;
	}

	//Check dataport
	dport=atoi(argv[4]);
	if(dport<20){
		printf("Incorrect data transfer port\n");
		return -1;
	}else if(dport==port){
		printf("FTP server port and data transfer port can not be same\n");
		return -1;
	}
		
	//Check address
	hp=gethostbyname(argv[6]);
	if(hp==NULL){
		printf("Incorrect FTP server address\n");
		return h_errno;
	}

	//Set IP-settings
	memset(&o,0,sizeof(struct sockaddr_in));
	o.sin_family=AF_INET;
	memcpy(&o.sin_addr.s_addr,hp->h_addr,hp->h_length);
	o.sin_port=htons(port);

	//Open datasocket
	dsfd=socket(AF_INET,SOCK_STREAM,0);
	if(dsfd<0){
		printf("Error while opening datasocket\n");
		return -2;
	}

	//Set datasocket IP-settings
	memset(&od,0,sizeof(struct sockaddr_in));
	od.sin_family=AF_INET;
	od.sin_addr.s_addr=INADDR_ANY;
	od.sin_port=htons(dport);

	//Allow reuse of datasocket
	n=1;
	if(setsockopt(dsfd,SOL_SOCKET,SO_REUSEADDR,(void*)&n,sizeof(n))==-1){
		perror("Error in setsockopt");
		return errno;
	}

	//Bind datasocket
	l=sizeof(od);
	n=bind(dsfd,(struct sockaddr*)&od,l);
	if(n<0){
		printf("Error in setsockopt\n");
		return errno;
	}
	
	//Listen datasocket
	n=listen(dsfd,10);
	if(n<0){
		perror("Error in listen function");
		return errno;
	}
	
       //Welcome Screen, interactivity
       printf("\n*******************************************************************");	
       printf("\t\n Welcome to File Transfer Protocol (FTP Client) based on RFC 959 ");
       printf("\t\n Write open,to open the connection. ");
       printf("\t\n Give default Username and Password ");
       printf("\t\n Client recognizes following commands- ");
       printf("\t\n Commands are: get, put, ls, cd, cd.., open, close, quit, passive");
       printf("\t\n Wish you have nice, File transfer session !!! ");
       printf("\n*******************************************************************\n");

	//Begin ftp-subprogram
	l=ftpfun(sfd,dsfd,dport,o);

	//Close socket only if ftp-subprogram returned an error
	if(l!=0){
		n=close(sfd);
		if(n<0){
			printf("Error while closing socket\n");
		}
	}
	
	//Close datasocket
	n=close(dsfd);
	if(n<0){
		printf("Error while cosing datasocket\n");
	}
	
	return l;
}

//FTP-subprogram
//Parameters: socket, datasocket, data transfer port, send address
int ftpfun(int sfd, int dsfd, int dport,struct sockaddr_in o){
	int n,continu=1,passive=0;
	fd_set rfds;
	char buf[2049];
	struct sockaddr_in da;
	
	//MAIN LOOP
	while(continu){
		memset(buf,0,2049);
		
		//Select-lists
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);

		n=select(1,&rfds,NULL,NULL,NULL);
		if(n<0){
			perror("Error in select function\n");
			return errno;
		}

		//Read keyboard command
		if(FD_ISSET(0,&rfds)){
			n=read(0,buf,2048);
			if(n<0){
				printf("Error while reading keyboard\n");
				return errno;
			}

			//Parse command
			n=parsfun(buf);
			switch(n){
				case GET: //Download
					if(sfd==-1){
						printf("No connection to server\n");
					}else{
						if(passive){
							dsfd=pdsocket(sfd,dsfd,&da);
							if(dsfd<0){
								return dsfd;
							}
						}

						n=getfun(sfd,dsfd,dport,buf,passive,da);
						if(n!=0){
							return n;
						}

						//Select new data transfer port
						dport++;
						if(dport==65536){
							dport=60000;
						}

						//Open new data transfer socket
						dsfd=dsoket(dsfd,dport,passive);
						if(dsfd<0){
							return dsfd;
						}
					}
					break;
					
				case PUT: //Upload
					if(sfd==-1){
						printf("No connection to server\n");
					}else{
						if(passive){
							dsfd=pdsocket(sfd,dsfd,&da);
							if(dsfd<0){
								return dsfd;
							}
						}

						n=putfun(sfd,dsfd,dport,buf,passive,da);
						if(n!=0){
							return n;
						}

						//Select new data transfer port
						dport++;
						if(dport==65536){
							dport=60000;
						}

						//Open new data transfer socket
						dsfd=dsoket(dsfd,dport,passive);
						if(dsfd<0){
							return dsfd;
						}
					}
					break;
					
				case LS: //Directory listing
					if(sfd==-1){
						printf("No connection to server\n");
					}else{
						if(passive){
							dsfd=pdsocket(sfd,dsfd,&da);
							if(dsfd<0){
								return dsfd;
							}
						}

						n=listfun(sfd,dsfd,dport,buf,passive,da);
						if(n<0){
							return n;
						}

						//Select new data transfer port
						dport++;
						if(dport==65536){
							dport=60000;
						}

						//Open new data transfer socket
						dsfd=dsoket(dsfd,dport,passive);
						if(dsfd<0){
							return dsfd;
						}
					}
					break;
					
				case OPEN:
					//Open socket if necessary
					if(sfd==-1){
						sfd=socket(AF_INET,SOCK_STREAM,0);
						if(sfd<0){
							printf("Error while opening socket\n");
							return -2;
						}
					}
					
					//Open connection to the server
					n=openfun(sfd,o);
					//1==error
					if(n==1){
						n=closefun(sfd);
						if(n!=0){
							return n;
						}else{
							//Mark socket as closed
							sfd=-1;
						}
					}else if(n<0){
						return n;
					}
					break;
					
				case CLOSE:
					//Close connection
					if(sfd==-1){
						printf("No connection to server\n");
					}else{
						n=closefun(sfd);
						if(n!=0){
							return n;
						}else{
							//Mark socket closed
							sfd=-1;
						}
					}
					break;
					
				case QUIT:
					//Quit and close socket
					if(sfd!=-1){
						n=closefun(sfd);
						if(n!=0){
							return n;
						}else{
							sfd=-1;
						}
					}
					continu=0;
					break;
					
				case CD: //Change directory
					if(sfd==-1){
						printf("No connection to server\n");
					}else{
						n=cdfun(sfd,buf);
						if(n!=0){
							return n;
						}
					}
					break;
					
				case CDUP: //Move one directory level up
					if(sfd==-1){
						printf("No connection to server\n");
					}else{
						n=cdupfun(sfd);
						if(n!=0){
							return n;
						}
					}
					break;

				case PASV: //Enter passive mode
					if(sfd==-1){
						printf("No contact with server\n");
					}else{
						if(!passive){
							passive=1;
							printf("Entering passive mode\n");
						}else{
							passive=0;
							printf("Entering active mode\n");
						}
					}
					break;

				default: //Error in command
					printf("Incorrect command\n");
			}
		}
		
	}
	
	return 0;
}

//Parsfun-subprogram
//Parses the command user gave
//Parameters: the command user gave
//Return value: 0 if error, positive value othervise
int parsfun(char* buf){
	if(!strncmp(buf,"get ",4)) return GET;
	else if(!strncmp(buf,"put ",4)) return PUT;
	else if((!strcmp(buf,"ls\n")||(!strncmp(buf,"ls ",3)))) return LS;
	else if(!strcmp(buf,"open\n")) return OPEN;
	else if(!strcmp(buf,"close\n")) return CLOSE;
	else if(!strcmp(buf,"quit\n")) return QUIT;
	else if((!strcmp(buf,"cd..\n"))||(!strcmp(buf,"cd ..\n"))) return CDUP;
	else if(!strncmp(buf,"cd ",3)) return CD;
	else if(!strcmp(buf,"passive\n")) return PASV;
	else return 0;
}

//Getfun-function
//Download desired file from server
//Parameters: socket, data transfer socket, data transfer port, command the user gave, passive mode
//Return value: 0 in normal case
int getfun(int sfd,int dsfd,int dport,char* par,int passive,struct sockaddr_in da){
	int i,dfd,fd,continu=1;
        unsigned int n;
	char buf[2049],temp[2049];
	struct hostent *hp;
	struct sockaddr_in o;

	//Remove the new line from end of the string
	par[strlen(par)-1]='\0';
	
	//Open file for writing
	fd=open(&par[4],O_WRONLY|O_CREAT|O_EXCL);
	if(fd<0){
		if(errno==EEXIST){
			printf("File already exist\n");
			return 0;
		}else{
			printf("Error while opening file\n");
			return errno;
		}
	}

	//Send TYPE I (binary)
	memset(buf,0,2049);
	sprintf(buf,"TYPE I\r\n");
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n<0){
		printf("Error in data transfer\n");
		return n;
	}

	//Receive acknowlegdement
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check receive code
	if(strncmp(buf,"200",3)){
		printf("TYPE-Type command failed\n");
		close(fd);
		return 0;
	}

	memset(buf,0,2049);
	memset(temp,0,2049);

	if(!passive){
		//Check home address
		n=gethostname(buf,2048);
		if(n<0){
			perror("Error in gethostname function");
			close(fd);
			return errno;
		}

		//Convert address to IP-number
		hp=gethostbyname(buf);
		if(hp==NULL){
			herror("Error in gethostbyname function\n");
			close(fd);
			return h_errno;
		}

		//Convert address to char-type and replace dots with commas (needed for the PORT-command)
		strcpy(temp,inet_ntoa(*((struct in_addr*)hp->h_addr)));
		for(i=0;i<strlen(temp);i++){
			if(temp[i]=='.'){
				temp[i]=',';
			}
		}

		//PORT <ip (comma separated)>,<floor(dataport/256)>,<dataport%256>
		memset(buf,0,2049);
		sprintf(buf,"PORT %s,%d,%d\r\n",temp,(int)floor(dport/256),(int)dport%256);
	
		//Send PORT-command
		printf("---> %s",buf);
		n=send(sfd,buf,strlen(buf),0);
		if(n!=strlen(buf)){
			printf("Error in data transfer\n");
			close(fd);
			return -1;
		}

		//Receive acknowledgement
		memset(buf,0,2049);
		n=receive(sfd,buf);
		if(n<0){
			close(fd);
			return n;
		}
	
		//Check that return code was 200
		if(strncmp(buf,"200",3)){
			printf("PORT-PORT command failed\n");
			close(fd);
			return 0;
		}
	}

	//Send RETR-command
	memset(buf,0,2049);
	sprintf(buf,"RETR%s\r\n",&par[3]);
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		close(fd);
		return -1;
	}

	//Open passive data connection
	if(passive){
		n=connect(dsfd,(struct sockaddr*)&da,sizeof(struct sockaddr_in));
		if(n<0){
			printf("Error in creating passive data connection\n");
			return n;
		}
		dfd=dsfd;
		printf("Passive data connection opened\n");
	}

	//Receive acknowledgement
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		close(fd);
		return n;
	}

	//Check that return code was 150, otherwise remove file
	if(strncmp(buf,"150",3)){
		n=remove(&par[4]);
		if(n<0){
			perror("Error while deleting file");
			return errno;
		}
		//If return code was 550 (file not found), return 0
		if(!strncmp(buf,"550",3)){
			return 0;
		}else{
			//Otherwise quit program
			printf("Error while loading file\n");
			return -1;
		}
	}

        //Open active data connection
	memset(buf,0,2049);
	if(!passive){
		n=sizeof(struct sockaddr_in);
		dfd=accept(dsfd,(struct sockaddr*)&o,&n);
		if(dfd<0){
			printf("Error while connecting data connection\n");
			close(fd);
			return -3;
		}
	}
	printf("Data connection created\n");
	
	//Receive file
	while(continu){
		memset(buf,0,2049);
		n=recv(dfd,buf,2048,0);
		if(n<0){
			printf("Error in data transfer\n");
			close(fd);
			close(dfd);
			return n;
		}else if(n==0){
			continu=0;
		}

		//Write buffer to disk
		n=write(fd,buf,n);
		if(n<0){
			perror("Error while wrting file");
			close(fd);
			close(dfd);
			return errno;
		}
	}

	//Close file
	n=close(fd);
	if(n<0){
		printf("Error while closing file\n");
		return n;
	}

	//Close data connection
	n=close(dfd);
	if(n<0){
		printf("Error while closing data connection\n");
		return n;
	}

	//Receive acknowledgement
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check that return code was 226
	if(strncmp(buf,"226",3)){
		printf("Error while loading file\n");
		return -1;
	}
	
	return 0;
}

//Putfun-subprogram
//Upload file to server 
//Parameters: socket, data transfer socket, data transfer port, command user gave, passive mode, server address if passive mode
//Return value: 0 in normal case
int putfun(int sfd,int dsfd,int dport,char* par,int passive,struct sockaddr_in da){
	int i,dfd,fd,continu=1,k;
        unsigned int n;
	char buf[2049],temp[2049];
	struct hostent *hp;
	struct sockaddr_in o;
	int size;
	double time;
	struct timeval begin,end;

	//Remove new line at the end
	par[strlen(par)-1]='\0';
	
	//Open file for reading
	fd=open(&par[4],O_RDONLY);
	if(fd<0){
		printf("File already exist\n");
		return 0;
	}
	
	//Send TYPE I (binary)
	memset(buf,0,2049);
	sprintf(buf,"TYPE I\r\n");
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n<0){
		printf("Error in data transfer\n");
		return n;
	}

	//Receive acknowlegdement
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check return code
	if(strncmp(buf,"200",3)){
		printf("TYPE-Type command failed\n");
		close(fd);
		return 0;
	}

	memset(buf,0,2049);
	memset(temp,0,2049);

	if(!passive){
		//Check home address
		n=gethostname(buf,2048);
		if(n<0){
			perror("Error in gethostname function");
			close(fd);
			return errno;
		}

		//Conver address to IP-number
		hp=gethostbyname(buf);
		if(hp==NULL){
			herror("Error in gethostname function\n");
			close(fd);
			return h_errno;
		}

		//Convert address to char-type and replace dots with commas
		strcpy(temp,inet_ntoa(*((struct in_addr*)hp->h_addr)));
		for(i=0;i<strlen(temp);i++){
			if(temp[i]=='.'){
				temp[i]=',';
			}
		}

		//PORT <ip (comma separated)>,<floor(port/256)>,<port%256>
		memset(buf,0,2049);
		sprintf(buf,"PORT %s,%d,%d\r\n",temp,(int)floor(dport/256),(int)dport%256);
	
		//Send PORT command
		printf("---> %s",buf);
		n=send(sfd,buf,strlen(buf),0);
		if(n!=strlen(buf)){
			printf("Error in data transfer\n");
			close(fd);
			return -1;
		}

		//Receive acknowlegdement
		memset(buf,0,2049);
		n=receive(sfd,buf);
		if(n<0){
			close(fd);
			return n;
		}
	
		//Check that return code was 200
		if(strncmp(buf,"200",3)){
			printf("PORT-PORT command failed\n");
			close(fd);
			return 0;
		}
	}

	//Send STOR-command
	memset(buf,0,2049);
	sprintf(buf,"STOR%s\r\n",&par[3]);
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		close(fd);
		return -1;
	}
	
	//Open passive data connection
	if(passive){
		n=connect(dsfd,(struct sockaddr*)&da,sizeof(struct sockaddr_in));
		if(n<0){
			printf("Error opening passive data connection\n");
			return n;
		}
		dfd=dsfd;
	}

	//Receive acknowlegdement
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		close(fd);
		return n;
	}

	//Check that return code was 150
	if(strncmp(buf,"150",3)){
		close(fd);
		return 0;
	}
	
        //Open active data connectionFTPClient.c
	memset(buf,0,2049);
	n=sizeof(struct sockaddr_in);
	if(!passive){
		dfd=accept(dsfd,(struct sockaddr*)&o,&n);
		if(dfd<0){
			printf("Error while connecting data connection\n");
			close(fd);
			return -3;
		}
		printf("Data connection created\n");
	}

	gettimeofday(&begin,NULL);
	size=0;

	//Send the file
	while(continu){
		memset(buf,0,2049);

		//Read buffer from file
		k=read(fd,buf,2048);
		if(k<1){
			perror("Error while reading file\n");
			close(fd);
			close(dfd);
			return errno;
		}else if(k<2048){
			continu=0;
		}
	
		//Calculate the size of the file
		size+=k;

		//Send the buffer
		n=send(dfd,buf,k,0);
		if(n!=k){
			printf("Error in data transfer\n");
			close(fd);
			close(dfd);
			return n;
		}
	}

         //Trying just now..
	gettimeofday(&end,NULL);

	time=end.tv_sec-begin.tv_sec;
	if(begin.tv_usec>end.tv_usec){
		time=time-1;
	}
	time+=((double)(end.tv_usec-begin.tv_usec))/1000000;

	printf("Transfer speed: %lf\n",(double)size/time);
	printf("Time taken: %lf\n",time);

	//Close file
	n=close(fd);
	if(n<0){
		printf("Error while closing file\n");
		return n;
	}

	//Close data connection
	n=close(dfd);
	if(n<0){
		printf("Error while closing data connection\n");
		return n;
	}

	//Receive ack
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check that return code was 226
	if(strncmp(buf,"226",3)){
		printf("Error while reading file\n");
		return -1;
	}
	
	return 0;
}

//Openfun-subprogram
//Open connection to server
//Parameters: socket, address
//Return value: 0 normally, 1 if sign in failed, negative in error
int openfun(int sfd,struct sockaddr_in o){
	int n,l,continu=1;
	char buf[2049],temp[2049];

	//Open connection to server
	l=sizeof(struct sockaddr_in);
	n=connect(sfd,(struct sockaddr*)&o,l);
	if(n<0){
		printf("No connection to server\n");
		return errno;
	}

	printf("Connection ready\n");
		
	//receive greeting
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}
	if(strncmp(buf,"220",3)){
		printf("Error while connecting to server\n");
		return -4;
	}

	//Ask for user name and password. Try tree times until give up.
	while(continu){
	
		//Give username
		memset(buf,0,2049);
		memset(temp,0,2049);
		printf("Give user name: ");
		scanf("%s",temp);
		sprintf(buf,"USER %s\r\n",temp);

		//Send USER-command
		printf("---> %s",buf);
		n=send(sfd,buf,strlen(buf),0);
		if(n!=strlen(buf)){
			printf("Error in data transfer\n");
			return -1;
		}

		//Receive ack
		memset(buf,0,2049);
		n=receive(sfd,buf);
		if(n<0){
			return n;
		}

		//Check that return code was 331
		if(strncmp(buf,"331",3)){
			printf("Error while sending user name\n");
			return -3;
		}
		
		//Give password
		memset(buf,0,2049);
		memset(temp,0,2049);
		printf("Give password: ");
		scanf("%s",temp);
		sprintf(buf,"PASS %s\r\n",temp);

		//Send PASS-command
		printf("---> %s",buf);
		n=send(sfd,buf,strlen(buf),0);
		if(n!=strlen(buf)){
			printf("Error in data transfer\n");
			return -1;
		}
	
		//Receive ack
		memset(buf,0,2049);
		n=receive(sfd,buf);
		if(n<0){
			return n;
		}

		//If return code was 230 end loop, otherwise ask user id again
		if(strncmp(buf,"230",3)){
			printf("Inlid password\n");
			continu++;

			//Quit after three failed tries
			if(continu>3){
				printf("Login failed!\n");
				return 1;
			}
		}else{
			continu=0;
		}
	
	}
	
	return 0;
}

//Closefun-subprogram
//close connection to server
//Parameters: socket
//Return value: 0 normally, negative in error
int closefun(int sfd){
	int n;
	char buf[2049];

	memset(buf,0,2049);
	sprintf(buf,"QUIT\r\n");

	//Send QUIT-command
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		return -1;
	}
	
	//receive ack
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}
	
	//Check that return code was 221
	if(strncmp(buf,"221",3)){
		printf("Error while closing connection\n");
		return -3;
	}
	
	//Close connection
	n=close(sfd);
	if(n<0){
		printf("Error while closing connection\n");
		return -2;
	}
	
	return 0;
}

//Cdfun-subprogram
//Change directory in server
//Parameters: socket, command user gave
//Return value: 0 normally, negative in error
int cdfun(int sfd,char* par){
	int n;
	char buf[2049];

	memset(buf,0,2049);
	
	//Remove new line at the end
	par[strlen(par)-1]='\0';
	
	//Send CWD-command
	sprintf(buf,"CWD %s\r\n",&par[3]);
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		return -1;
	}

	//Receive ack
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}
	
	return 0;
}

//Cdupfun-subprogram
//Change directory to one level up
//Parameters: socket
//return value: 0 normally, negative in error
int cdupfun(int sfd){
	int n;
	char buf[2049];
	
	memset(buf,0,2049);
	
	//Send CDUP-command
	sprintf(buf,"CDUP\r\n");
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		return -1;
	}
	
	//Receive ack
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}
	
	return 0;
}

//Lsfun-subprogram
//Get directory listing from server
//Parameters: socket, data transfer socket, data transfer port, command user gave, passive mode, server address if passive mode
//Return value: 0 normally, negative in error
int listfun(int sfd,int dsfd,int dport,char* par,int passive,struct sockaddr_in da){
	int i,dfd;
        unsigned int n;
	char buf[2049],temp[2049];
	struct hostent *hp;
	struct sockaddr_in o;
	
	memset(buf,0,2049);
	memset(temp,0,2049);

	//Send TYPE A -command (ASCII)
	sprintf(buf,"TYPE A\r\n");
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		return -1;
	}

	//Receive ack
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check that return code was 200
	if(strncmp(buf,"200",3)){
		printf("TYPE-Type command failed\n");
		return 0;
	}
	
	if(!passive){
		//Check host address
		n=gethostname(buf,2048);
		if(n<0){
			perror("Error in gethostname function");
			return errno;
		}

		//Convert address to IP-number
		hp=gethostbyname(buf);
		if(hp==NULL){
			herror("Error in gethostbyname function\n");
			return h_errno;
		}

		//Convert address to char-type and replace dots with commas
		strcpy(temp,inet_ntoa(*((struct in_addr*)hp->h_addr)));
		for(i=0;i<strlen(temp);i++){
			if(temp[i]=='.'){
				temp[i]=',';
			}	
		}

		//PORT <ip (comma separated)>,<floor(port/256)>,<port%256>
		memset(buf,0,2049);
		sprintf(buf,"PORT %s,%d,%d\r\n",temp,(int)floor(dport/256),(int)dport%256);
	
		//Send PORT-command
		printf("---> %s",buf);
		n=send(sfd,buf,strlen(buf),0);
		if(n!=strlen(buf)){
			printf("Error in data transfer\n");
			return -1;
		}

		//Receive ack
		memset(buf,0,2049);
		n=receive(sfd,buf);
		if(n<0){
			return n;
		}
	
		//check that return code was 200
		if(strncmp(buf,"200",3)){
			printf("PORT-PORT command failed\n");
			return 0;
		}
	}

	//Remove new line at the end
	par[strlen(par)-1]='\0';
	
	//Send LIST-command
	memset(buf,0,2049);
	sprintf(buf,"LIST%s\r\n",&par[2]);
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		return -1;
	}
	
	//Open passive data connection
	if(passive){
		n=connect(dsfd,(struct sockaddr*)&da,sizeof(struct sockaddr_in));
		if(n<0){
			printf("Error opening passive data connection\n");
			return n;
		}
		dfd=dsfd;
	}

	//Receive ack
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}
	
	//Check that return code 150
	if(strncmp(buf,"150",3)){
		printf("LIST-LIST command failed\n");
		return 0;
	}
	
	//Open active data connection
	if(!passive){
		n=sizeof(struct sockaddr_in);
		dfd=accept(dsfd,(struct sockaddr*)&o,&n);
		if(dfd<0){
			printf("Error while connecting data connection\n");
			return -3;
		}
		printf("Data connection created\n");
	}

	//Receive directory listing
	memset(buf,0,2049);
	n=receive(dfd,buf);
	if(n<0){
		return n;
	}

	//Close data connection
	n=close(dfd);
	if(n<0){
		printf("Error while closing connection\n");
		return -3;
	}
	
	//Receive message confirming the closure of data connection
	memset(buf,0,2049);
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check that return code was 226
	if(strncmp(buf,"226",3)){
		printf("Error");
		return -4;
	}
	
	return 0;
}

//Passivefun-subprogram
//Sets the connection to passive mode
//Parameters: socket, pointer for a sockaddr_in struct where the received address and port are stored
//Return value: 0 normally, negative in error
int passivefun(int sfd,struct sockaddr_in *da){
	int n,i,port;
	char buf[2049],buf2[2049],*p;

	memset(buf,0,2049);
	memset(buf2,0,2049);

	//Send PASV-command
	sprintf(buf,"PASV\r\n");
	printf("---> %s",buf);
	n=send(sfd,buf,strlen(buf),0);
	if(n!=strlen(buf)){
		printf("Error in data transfer\n");
		return -1;
	}

	//Receive ack
	n=receive(sfd,buf);
	if(n<0){
		return n;
	}

	//Check that return code was 227
	if(strncmp(buf,"227",3)){
		printf("Error entering passive mode\n");
		return -2;
	}

	da->sin_family=AF_INET;

	//Copy the address part of the returned message
	p=strchr(buf,'(');
	p=strchr(p+1,',');
	p=strchr(p+1,',');
	p=strchr(p+1,',');
	p=strchr(p+1,',');

	strncpy(buf2,strchr(buf,'(')+1,p-strchr(buf,'(')-1);

	for(i=0;i<strlen(buf2);i++){
		if(buf2[i]==','){
			buf2[i]='.';
		}
	}

	//Convert the address from char-type
	n=inet_aton(buf2,&da->sin_addr);
	if(!n){
		printf("Error converting address\n");
		return -3;
	}

	memset(buf2,0,2049);

	//Convert port
	strncpy(buf2,p+1,strchr(p+1,',')-p-1);
	port=atoi(buf2)*256;
	p=strchr(p+1,',');
	memset(buf2,0,2049);
	strncpy(buf2,p+1,strchr(p+1,')')-p-1);
	port+=atoi(buf2);
	da->sin_port=htons(port);

	return 0;
}

//Dsocket-subprogram
//Open new data transfer socket
//Parameters: data transfer socket, data transfer port
//Return value: positive value normally, negative in error
int dsoket(int dsfd,int dport,int passive){
	struct sockaddr_in o;
	int n,l;

	if(!passive){
		//Close old socket
		n=close(dsfd);
		if(n<0){
			printf("Error while closing socket\n");
			return n;
		}
	}

	//Open new socket
	dsfd=socket(AF_INET,SOCK_STREAM,0);
	if(dsfd<0){
		printf("Error while opening socket\n");
		return dsfd;
	}
	
        //Set IP-settings
	memset(&o,0,sizeof(struct sockaddr_in));
	o.sin_family=AF_INET;
	o.sin_addr.s_addr=INADDR_ANY;
	o.sin_port=htons(dport);

	//Allow socket reuse
	n=1;
	if(setsockopt(dsfd,SOL_SOCKET,SO_REUSEADDR,(void*)&n,sizeof(n))==-1){
		perror("Error in setsockopt");
		return -1;
	}

	//Bind socket
	l=sizeof(o);
	n=bind(dsfd,(struct sockaddr*)&o,l);
	if(n<0){
		printf("Error while binding socket\n");
		return -1;
	}
	
	//Listen to the socket
	n=listen(dsfd,10);
	if(n<0){
		perror("Error in listen function");
		return -1;
	}
	
	return dsfd;
}

//Pdsocket-subprogram
//Open a new passive data transfer socket
//Parameters: socket, data transfer socket, server address
//Return value: positive value normally, negative in error
int pdsocket(int sfd,int dsfd, struct sockaddr_in *da){
	int n;
	struct sockaddr_in da2;

	//Close old socket
	n=close(dsfd);
	if(n<0){
		printf("Error closing socket\n");
		return n;
	}

	//Open a new socket
	dsfd=socket(AF_INET,SOCK_STREAM,0);
	if(dsfd<0){
		printf("Error opening socket\n");
		return dsfd;
	}

	//Send PASV-command
	n=passivefun(sfd,da);
	if(n<0){
		return n;
	}

	//Allow socket reuse
	n=1;
	if(setsockopt(dsfd,SOL_SOCKET,SO_REUSEADDR,(void*)&n,sizeof(n))==-1){
		perror("Error in setsockopt()");
		return -1;
	}

	//Set IP-settings
	da2.sin_family=AF_INET;
	da2.sin_addr.s_addr=INADDR_ANY;
	da2.sin_port=htons(55555);

	//Bind socket
	n=bind(dsfd,(struct sockaddr*)&da2,sizeof(struct sockaddr_in));
	if(n<0){
		perror("Error in binding socket\n");
		return n;
	}

	return dsfd;
}

//receive-subprogram
//Receive a message from server
//Parameters: socket, buffer (only return code is stored)
//Return code: 1 normally, negative in error
int receive(int sfd,char *retval){
	int n,i,continu=1,pal=0;
	char buf[2049],*p;

	//Receive until entire message is received
	while(continu){
		//Receive data
		memset(buf,0,2049);
		n=recv(sfd,buf,2048,0);
		if(n<0){
			printf("Error in data transfer\n");
			return n;
		}else if(n==0){
			printf("Connection closed by server\n");
			return -2;
		}

		//Is there \r\n at the end?
		for(i=1;i<strlen(buf);i++){
			if((buf[i-1]=='\r')&&(buf[i]=='\n')) continu=0;
		}

		//Does the last return code have "-" at the end?
		p=buf;
		while(strchr(p,'\n')!=strrchr(buf,'\n')){
			p=strchr(p+1,'\n');
		}
		if(p[3]=='-') continu=1;

		//If buffer is full continue
		if(n==2048) continu=1;
		
		//Only the three first characters are returned (return code number) EXCEPT if return code is 227 (passive mode accept)
		if(!pal){
			strncpy(retval,buf,3);
			pal=0;

			if(!strncmp(buf,"227",3)){
				strcpy(retval,buf);
			}
		}

		//Print buffer
		printf(buf);
	}
	
	return 1;
}
// This is the end of home exam (ftp client based on RFC 959)
// Comments and feedback at        santosh (dot) kalwar (at) lut (dot) fi
