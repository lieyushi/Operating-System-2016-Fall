// Obligatory includes 
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string.h>
#include "header.h"

using namespace std;

// Constants
#define ALPHA 0
#define BRAVO 5

void sendSever(const Message* message, Answer* reply);
const int interactive(const int& operation, const int& resource);

// Templates to be filled
int create_lock(int resource)
{  
	return interactive(CREATE_LOCK, resource);
}


int read_lock(int resource)
{  
	return interactive(READ_LOCK, resource);
}

int write_lock(int resource)
{
	return interactive(WRITE_LOCK, resource);
}

int read_unlock(int resource)
{
	return interactive(READ_UNLOCK, resource);
} 


int write_unlock(int resource)
{
	return interactive(WRITE_UNLOCK, resource);
}

int delete_lock(int resource)
{
	return interactive(DELETE_LOCK, resource);
}

int kill_server()
{
	return interactive(KILL_SERVER, 0);
}

int main () {
    int pid; // child's pid

        // Before the fork
        cout << "Create lock ALPHA\n";
    create_lock(ALPHA);
        //sleep(1);
        cout << "Create lock BRAVO\n";
    create_lock(BRAVO);
        //sleep(1);
        cout << "Parent requests write permission on lock BRAVO\n";
    write_lock(BRAVO);
        //sleep(1);
        cout << "Write permission on lock BRAVO was granted\n";
        cout << "Parent requests read permission on lock ALPHA\n";
    read_lock(ALPHA);
        cout << "Read permission on lock ALPHA was granted\n";
    sleep(1);
    
    // Fork a child
    if ((pid = fork()) == 0) {
        // Child process
            cout << "Child requests read permission on lock ALPHA\n";
        read_lock(ALPHA); // This permission should be granted
            cout << "Read permission on lock ALPHA was granted\n";
        sleep(1);
            cout << "Child releases read permission on lock ALPHA\n";
        read_unlock(ALPHA);
        sleep(1);
            cout << "Child requests write permission on lock BRAVO\n";
        write_lock(BRAVO); // Should wait until parent relases its lock
            cout << "Write permission on lock BRAVO was granted\n";
        sleep(1);
            cout << "Child releases write permission on lock BRAVO\n";
        write_unlock(BRAVO);
        cout << "Child terminates\n";
                _exit(0);
    } // Child

    // Back to parent
        cout << "Parent releases read permission on lock ALPHA\n";
    read_unlock(ALPHA);
        //sleep(1);
        cout << "Parent requests write permission on lock ALPHA\n";
    write_lock(ALPHA); // Should wait until child removes its read lock
        cout << "Write permission on lock ALPHA was granted\n";
    sleep(1);
        cout << "Parent releases write permission on lock ALPHA\n";
    write_unlock(ALPHA);
    sleep(1);
        cout << "Parent releases write permission on lock BRAVO\n";
    write_unlock(BRAVO);

    // Child and parent join
        while (pid != wait(0));  // Busy wait
    delete_lock(ALPHA);
        delete_lock(BRAVO);
        // We assume that failed operations return a non-zero value
        if (write_lock(ALPHA) != 0) {
        cout << "Tried to access a deleted lock\n";
    }
    kill_server();
    return 0;
} // main

void sendSever(const Message* message, Answer* reply)
{
	int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket!\n");
        exit(-1);
    }
    server = gethostbyname(LOCALHOST);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host!\n");
        exit(-1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(PORT);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        perror("ERROR connecting!\n");
        exit(-1);
    }

    n = write(sockfd,message,sizeof(Message));
    if (n < 0) 
    {
         perror("ERROR writing to socket!\n");
         exit(-1);
    }
    bzero(reply,MAXSIZE);
    //char buffer[MAXSIZE];
    n = read(sockfd,reply,sizeof(Answer));
    if (n < 0) 
    {
         perror("ERROR reading from socket");
         exit(-1);
    }
    close(sockfd);
    return;
}
	
const int interactive(const int& operation, const int& resource)
{
	Message *mess = (Message*)malloc(sizeof(Message));
	Answer *ans = (Answer*)malloc(sizeof(Answer));
	bzero(mess,sizeof(Message));
	bzero(ans,sizeof(Answer));
	mess->operation = operation;
	mess->number = resource;
	sendSever(mess, ans);
	int solution =  ans->solution;
	printf("%s\n\n", ans->reply);
	free(ans);
    free(mess);
    //printf("%d\n", solution);
	return solution;
}
