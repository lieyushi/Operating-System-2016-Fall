#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <semaphore.h>

using namespace std;
sem_t *access_ = NULL;
pthread_mutex_t gMutex;
int plaza = 0;
int Status = 0;
const char* mutex_name = "semaphore";
vector<string> holder;

struct Guest
{
	string tag, name;
	int arrival, waiting;
	Guest(string& line)
	/* parse information from string to object */
	{
		string temp;
		stringstream ss(line);
		getline(ss, tag, ' ');
		getline(ss, name, ' ');
		getline(ss, temp, ' ');
		arrival = atoi(temp.c_str());
		getline(ss, temp, ' ');
		waiting = atoi(temp.c_str());
		ss.clear();
	};
};

void *PlazaControl(void *threadid);
void readGuest(const char* fileName, vector<Guest*>& people);
void initSemaphore();
void killSempahore();
int getValue(const Guest* guest);
void getHolder(const vector<Guest*>& people);

int main (int argc, char *argv[])
{
	if(argc != 2)
	{
		perror("Argument number is wrong!\n");
		perror("You must have execute file!\n");
		exit(-1);
	}
	vector<Guest*> people;

	readGuest(argv[1], people);
	getHolder(people);
	initSemaphore();
	const int NUM_THREADS = people.size();
	pthread_t threads[NUM_THREADS];
	int rc;
	int i;
	
	for( i=0; i < NUM_THREADS; i++ )
	{
		  rc = pthread_create(&threads[i], NULL, 
		                      PlazaControl, (void *)people[i]);
		  if (rc)
		  {
		     cout << "Error:unable to create thread," << rc << endl;
		     exit(-1);
	  	  }
	}

	for (int i = 0; i < NUM_THREADS; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	cout << endl;
	people.clear();
	killSempahore();
	pthread_exit(NULL);
	return 0;
}

void readGuest(const char* fileName, vector<Guest*>& people)
{
	ifstream fin(fileName);
	if(!fin)
	{
		perror("Can't find the file in current directory!\n");
		exit(-1);
	}
	string line;
	while(getline(fin, line))
	{
		people.push_back(new Guest(line));
	}
	for (int i = 0; i < holder.size(); ++i)
	{
		cout << holder[i] << " ";
	}
	cout << endl;
	fin.close();
}

void *PlazaControl(void *threadid)
{
	Guest *guest;
	guest = (Guest*)threadid;
	sleep(guest->arrival);
	cout << guest->tag << " " << guest->name << " arrives at time " << guest->arrival << endl;

	const int& value = getValue(guest);
	//cout << value << " " << Status << endl;
	if(Status!=0 && value!=Status)
		sem_wait(access_);
	
	pthread_mutex_lock(&gMutex);	
	cout << guest->tag << " " << guest->name << " enters the plaza" << endl;
	plaza++;
	Status = value;
	pthread_mutex_unlock(&gMutex);

	sleep(guest->waiting);

	pthread_mutex_lock(&gMutex);
	plaza--;
	cout << guest->tag << " " << guest->name << " leaves the plaza" << endl;
	if(plaza == 0)
	{	
		Status=0;
		sem_post(access_);
	}
	pthread_mutex_unlock(&gMutex);

	pthread_exit(NULL);
}

void initSemaphore()
{
	if((access_ = sem_open(mutex_name, O_CREAT, 0644, 0)) == SEM_FAILED)
	{
		perror("semaphore creating is wrong!\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&gMutex, NULL) != 0)
	{
		perror("mutex creating error!\n");
		exit(EXIT_FAILURE);
	}
}

void killSempahore()
{
	if(sem_close(access_) == -1)
	{
		perror("sem_close error!\n");
		exit(EXIT_FAILURE);
	}

	if(sem_unlink(mutex_name) !=0)
	{
		perror("Semaphore not delted completely!\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_destroy(&gMutex) != 0)
	{
		perror("mutex destroying error!\n");
		exit(EXIT_FAILURE);
	}
	holder.clear();
}

void getHolder(const vector<Guest*>& people)
{
	for (int i = 0; i < people.size(); ++i)
	{
		if(std::find(holder.begin(), holder.end(), people[i]->tag)==holder.end())
			holder.push_back(people[i]->tag);
	}
}

int getValue(const Guest* guest)
{
	for (int i = 0; i < holder.size(); ++i)
	{
		if(strcmp(guest->tag.c_str(), holder[i].c_str())==0)
			return i+1;;
	}
}
