#ifndef __HEADER_H__
#define __HEADER_H__

#define NETHOST (const char*)"127.0.0.1"
#define LOCALHOST (const char*)"localhost"
#define MAXSIZE 1024

#define CREATE_LOCK 1
#define READ_LOCK 2
#define WRITE_LOCK 3
#define READ_UNLOCK 4
#define WRITE_UNLOCK 5
#define DELETE_LOCK 6
#define KILL_SERVER 7

#include <queue>

const int PORT = 51717;

typedef struct 
{
	int operation;
	int number;
}Message;


typedef struct 
{
	char reply[MAXSIZE];
	int solution;
}Answer;

#define FREE 0
#define WRITE 1
#define READ 2

#define SUCCESS 0
#define FAIL 1

typedef struct 
{
	int index;
	int state;
	std::queue<int> writeQueue;
	std::queue<int> readQueue;
}Resource;

#endif