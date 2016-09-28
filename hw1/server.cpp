/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include <iostream>
#include <string.h>
#include "header.h"

using namespace std;

#define MAX_LOCK 32

void createLock();
void error(const char *msg);
void initServer(int &sockfd);
void getMessage(int &sockfd, int& newsockfd, struct sockaddr_in& cli_addr,
                socklen_t& clilen, int &n, Message *mess);
const bool handleOperation(const int& newsockfd, const Message* mess, Answer* ans, int& n);  
void sendReply(const int& newsockfd, Answer* ans, int& n);
void displayResource();

vector<Resource> resourceVec;
bool kill = false;

int main(int argc, char *argv[])
{
     int sockfd, newsockfd;
     struct sockaddr_in cli_addr;
     socklen_t clilen = sizeof(cli_addr);
     
     Message* mess = (Message*)malloc(sizeof(Message));
     Answer* ans = (Answer*)malloc(sizeof(Answer));
     int n;

     initServer(sockfd);

     while(1)
     {
        memset(mess,0,sizeof(Message));
        memset(ans,0,sizeof(Answer));
        getMessage(sockfd, newsockfd, cli_addr, clilen, n, mess);
        if(handleOperation(newsockfd, mess, ans, n))
        {
            sendReply(newsockfd,ans,n);
            close(newsockfd);
        }
        displayResource();
        if(kill)
            break;
     }
     free(mess);
     free(ans);
     close(sockfd);
     resourceVec.clear();
     return 0; 
}

void initServer(int &sockfd)
{   
    struct sockaddr_in serv_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     { 
        error("ERROR opening socket");
        exit(-1);
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(PORT);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
     {
              error("ERROR on binding");
              exit(-1);
     }
     if(listen(sockfd,5)<0)
     {
        error("Error server listens!\n");
        exit(-1);
     }
     return;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


void getMessage(int &sockfd, int& newsockfd, struct sockaddr_in& cli_addr, socklen_t& clilen, int &n,
                Message *mess)
{
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
    {
        error("ERROR on accept");
        exit(-1);
    }
    bzero(mess,sizeof(Message));
    n = read(newsockfd,mess,MAXSIZE);
    if (n < 0) 
    {
        error("ERROR reading from socket");
        exit(-1);
    }
    return;
}


const bool handleOperation(const int& newsockfd, const Message* mess, Answer* ans, int& n)
{
    bool result = false;
    bool find_ = false;
    int sockfd;
    switch(mess->operation)
    {
    case CREATE_LOCK:
        if(resourceVec.size() < MAX_LOCK)
        {
            Resource res = {mess->number, FREE, std::queue<int>(), std::queue<int>()};
            resourceVec.push_back(res);
            memcpy(ans->reply, "Lock created!",MAXSIZE);
            ans->solution = SUCCESS;
        }
        else
        {
            memcpy(ans->reply, "Lock exceeds 32!",MAXSIZE);
            ans->solution = FAIL;
        }
        result = true;
        printf("Create lock on Resource %d\n", mess->number);
        break;

    case READ_LOCK:
        if(!resourceVec.empty())
        {
            for (int i = 0; i < resourceVec.size(); ++i)
            {
                if(resourceVec[i].index==mess->number)
                {
                    find_ = true;
                    if(resourceVec[i].state == WRITE)
                    {
                        resourceVec[i].readQueue.push(newsockfd);
                    }
                    else
                    {
                        resourceVec[i].state = READ;
                        memcpy(ans->reply, "Read granted!",MAXSIZE);
                        ans->solution = SUCCESS;
                        result = true;
                    }
                    break;
                }
            }
        }
        if(!find_)
        {
            memcpy(ans->reply, "This resource doesn't exist!",MAXSIZE);
            ans->solution = FAIL;
            result = true;
        }
        break;

    case WRITE_LOCK:
        if(!resourceVec.empty())
        {
            for (int i = 0; i < resourceVec.size(); ++i)
            {
                if(resourceVec[i].index==mess->number)
                {
                    find_ = true;
                    if(resourceVec[i].state == FREE)
                    {
                        memcpy(ans->reply, "Write granted!", MAXSIZE);
                        ans->solution = SUCCESS;
                        resourceVec[i].state = WRITE;
                        result = true;
                    }
                    else
                    {
                        resourceVec[i].writeQueue.push(newsockfd);
                    }
                    break;
                }
            }
        }
        if(!find_)
        {
            memcpy(ans->reply, "This resource doesn't exist!",MAXSIZE);
            ans->solution = FAIL;
            result = true;
        }
        break;

    case READ_UNLOCK:
        if(!resourceVec.empty())
        {
            for (int i = 0; i < resourceVec.size(); ++i)
            {
                if(resourceVec[i].index==mess->number)
                {
                    find_ = true;
                    if(resourceVec[i].state == WRITE)
                    {
                        memcpy(ans->reply, "Read unlock fails due to write lock", MAXSIZE);
                        ans->solution = FAIL;
                        result = true;
                    }
                    else
                    {

                        if(resourceVec[i].readQueue.empty()&&resourceVec[i].writeQueue.empty())
                        {
                            resourceVec[i].state = FREE;
                        }
                        else if(!resourceVec[i].readQueue.empty())
                        {
                            while(!resourceVec[i].readQueue.empty())
                            {
                                sockfd = resourceVec[i].readQueue.front();
                                resourceVec[i].readQueue.pop();
                                ans->solution = SUCCESS;
                                memcpy(ans->reply, "Read lock granted!", MAXSIZE);
                                sendReply(sockfd, ans, n);
                                close(sockfd);
                            }
                            resourceVec[i].state = READ;
                        }
                        else if(!resourceVec[i].writeQueue.empty())
                        {
                            while(!resourceVec[i].writeQueue.empty())
                            {
                                sockfd = resourceVec[i].writeQueue.front();
                                resourceVec[i].writeQueue.pop();
                                ans->solution = SUCCESS;
                                memcpy(ans->reply, "Write lock granted!", MAXSIZE);
                                sendReply(sockfd, ans, n);
                                close(sockfd);
                            }
                            resourceVec[i].state = WRITE;
                        }

                        ans->solution = SUCCESS;
                        memcpy(ans->reply, "Read unlock granted!", MAXSIZE);
                        result = true;

                    }
                    break;
                }
            }
        }
        if(!find_)
        {
            memcpy(ans->reply, "This resource doesn't exist!",MAXSIZE);
            ans->solution = FAIL;
            result = true;
        }
        break;

    case WRITE_UNLOCK:
        if(!resourceVec.empty())
        {
            for (int i = 0; i < resourceVec.size(); ++i)
            {
                if(resourceVec[i].index==mess->number)
                {
                    find_ = true;
                    if(resourceVec[i].writeQueue.empty()&& resourceVec[i].readQueue.empty())
                    {
                        resourceVec[i].state = FREE;
                    }
                    else if(!resourceVec[i].writeQueue.empty())
                    {
                        while(!resourceVec[i].writeQueue.empty())
                        {
                            sockfd = resourceVec[i].writeQueue.front();
                            resourceVec[i].writeQueue.pop();
                            ans->solution = SUCCESS;
                            memcpy(ans->reply, "Write granted!", MAXSIZE);
                            sendReply(sockfd,ans,n);
                            close(sockfd);
                        }
                        resourceVec[i].state = WRITE;
                    }
                    else if(!resourceVec[i].readQueue.empty())
                    {
                        while(!resourceVec[i].readQueue.empty())
                        {
                            sockfd = resourceVec[i].readQueue.front();
                            resourceVec[i].readQueue.pop();
                            ans->solution = SUCCESS;
                            memcpy(ans->reply, "Read granted!", MAXSIZE);
                            sendReply(sockfd,ans,n);
                            close(sockfd);
                        }
                            resourceVec[i].state = READ;
                    }

                    memcpy(ans->reply, "Write unlock granted!", MAXSIZE);
                    ans->solution = SUCCESS;
                    result = true;

                    break;
                }
            }
        }
        if(!find_)
        {
            memcpy(ans->reply, "This resource doesn't exist!",MAXSIZE);
            ans->solution = FAIL;
            result = true;
        }
        break;

    case DELETE_LOCK:
        if(!resourceVec.empty())
        {
            for (int i = 0; i < resourceVec.size(); ++i)
            {
                if(resourceVec[i].index==mess->number)
                {
                    find_ = true;
                    if(resourceVec[i].state != FREE)
                    {
                        ans->solution = FAIL;
                        memcpy(ans->reply, "Delete fails due to being busy!", MAXSIZE);
                    }
                    else
                    {
                        resourceVec.erase(resourceVec.begin()+i);
                        ans->solution = SUCCESS;
                        memcpy(ans->reply, "Delete success!", MAXSIZE);
                    }
                    result = true;
                    break;  
                }
            }
        }
        if(!find_)
        {
            memcpy(ans->reply, "This resource doesn't exist!",MAXSIZE);
            ans->solution = FAIL;
            result = true;
        }
        printf("Lock on Resource %d deleted!\n", mess->number);
        break;

    case KILL_SERVER:
        kill = true;
        memcpy(ans->reply, "Server killed!",MAXSIZE);
        ans->solution = SUCCESS;
        result = true;
        break;

    default:
        error("Can't read operation!\n");
        exit(-1);
        break;
    }
    
    return result;
}


void sendReply(const int& newsockfd, Answer* ans, int& n)
{   
    assert(ans!=NULL);
    n = write(newsockfd, ans, sizeof(Answer));
    if (n < 0) 
    {
        error("ERROR writing to socket");
        exit(-1);
    }
    return;
}

void displayResource()
{
    if(!resourceVec.empty())
    {
        for (int i = 0; i < resourceVec.size(); ++i)
        {
            char buffer[MAXSIZE];
            switch(resourceVec[i].state)
            {
            case 0:
                memcpy(buffer, "FREE",MAXSIZE);
                break;
            case 1:
                memcpy(buffer, "WRITE",MAXSIZE);
                break;
            case 2:
                memcpy(buffer, "READ",MAXSIZE);
                break;
            default:
                perror("Can't recognize!\n");
                exit(-1);
                break;
            }
            printf("Resource %d : %s\n", resourceVec[i].index, buffer);
        }
        printf("\n");
    }
    return;
}