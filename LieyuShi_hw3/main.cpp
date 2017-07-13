#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h> 
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

class backgroundProcess
{
public:
     int processID;
     char *command;

     backgroundProcess(int processID, char *line)
     {
          this->processID = processID;
          command = new char[100];
          memcpy(command, line, strlen(line));
     }

};


vector<int> programCount; /* record | */
vector<int> redirectionCount;
vector<char*> path; /* find path route for $PATH */
bool interaction = false; /* wait for completion */
int TYPE;
vector<backgroundProcess> background;


void checkBinary(char** argv);
void parse(char *line, char **argv, int& number, bool& interaction);
void execute(char **argv, const bool& interaction, const int& number, char* line);
void findPath();
void reset(int& number, char** argv_, char* line);
void reset(int& number, char* line);
void checkIllInput(char **argv, bool& found);
void setType();
void findBinary(char **argv_, const int& number); 
void performFirstType(char **argv, const bool& interaction, const int& number, char* line);
void performSecondType(char **argv, const bool& interaction, const int& number, char* line);
void performThirdType(char **argv, const bool& interaction, const int& number, char* line);
void performFourthType(char **argv, const bool& interaction, const int& number, char* line);
void increaseMomery(char **argv_, const int& number);
void getRedirection(char **argv, const int& index);
void getTempv(char **temp, char ***tempv, char **argv, const int& start, const int& size);
void allTerminate();
void printBackground();



int  main(int argc, char *argv[])
{
     char  line[1024];             
     char  *argv_[64];             
     findPath();
     while (1) 
     {  
          int number = 0;
          bool found = true;                
          printf("MyShell -> ");   
          gets(line);             
          printf("\n");

          parse(line, argv_, number, interaction);

          if (strcmp(line, "exit") == 0)
          { 
               allTerminate();
               exit(-1);
          }
          else if(strcmp(line, "processes") == 0)
          {
               printBackground();
               
               continue;
          }

          checkIllInput(argv_, found);
          if(found)
          {
               increaseMomery(argv_, number);
               setType(); 
               findBinary(argv_, number);
               execute(argv_, interaction, number, line);       
               reset(number, argv_, line);   
          }
          else
               reset(number, line);
     }

     return 0;
}


void parse(char *line, char **argv, int& number, bool& interaction)
{   
     char* pch = strchr(line, '&');
     if(pch != NULL)
     {
          interaction = true; 
          line[pch-line] = '\0';
          line[pch-line+1] = '\0';
     }
     else
          interaction = false;

     pch = strchr(line, ' ');
     if(pch == NULL)
     {
          char temp[50];
          int current = 0;
          for (int i = 0; i < strlen(line); ++i)
          {
               if(line[i]=='|' || line[i]=='<' || line[i]=='>')
               {
                    temp[current++] = ' ';
                    temp[current++] = line[i];
                    temp[current] = ' ';
               }
               else
                    temp[current] = line[i];
               current++;
          }
          temp[current] = '\0';
          memcpy(line, temp, strlen(temp));
     }

     while (*line != '\0') 
     {    
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';  
          if(*line != '\0')  
          {
               *argv++ = line;       
               while (*line != '\0' && *line != ' ' && 
                      *line != '\t' && *line != '\n') 
               {
                    if(*line == '|')
                         programCount.push_back(number);
                    else if(*line == '<' || *line == '>')
                         redirectionCount.push_back(number);
                    line++;
               } 
               number++;
          }
     }
     *argv = NULL;
}

void  execute(char **argv, const bool& interaction, const int& number, char* line)
{
     switch(TYPE)
     {
     case 0:
          performFirstType(argv, interaction, number, line);
          break;

     case 1:
          performSecondType(argv, interaction, number, line);
          break;

     case 2:
          performThirdType(argv, interaction, number, line);
          break;

     case 3:
          performFourthType(argv, interaction, number, line);
          break;
     }
}


void performFirstType(char **argv, const bool& interaction, const int& number, char* line)
{
     pid_t pid;

     char* temp = NULL;
     char** tempv = NULL;

     if ((pid = fork()) < 0) 
     {    
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }

     else if (pid == 0) 
     {

          getTempv(&temp, &tempv, argv, 0, number);

          if (execv(temp, tempv) < 0) 
          {    
               printf("*** ERROR: exec failed\n");
               exit(1);
          }

          if(tempv)
               delete[] tempv;

          exit(0);
     }

     if(interaction)
     {
          backgroundProcess bp(pid, line);
          background.push_back(bp);

     }
     else                            
          while (wait(0) != pid); 
}


void performSecondType(char **argv, const bool& interaction, const int& number, char* line)
{
     pid_t pid;

     char* temp = NULL;
     char** tempv = NULL;

     if ((pid = fork()) < 0) 
     {    
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }

     else if (pid == 0) 
     { 

          getTempv(&temp, &tempv, argv, 0, redirectionCount[0]);

          getRedirection(argv, redirectionCount[0]);       

          if (execv(temp, tempv) < 0) 
          {    
               printf("*** ERROR: exec failed\n");
               exit(1);
          }

          if(tempv)
               delete[] tempv;

          exit(0);
     }
   
     if(interaction)
     {
          backgroundProcess bp(pid, line);
          background.push_back(bp);
     }

     else                            
          while (wait(0) != pid); 
}

void performThirdType(char **argv, const bool& interaction, const int& number, char *line)
{
     pid_t pid;

     char* temp = NULL;
     char** tempv = NULL;

     if ((pid = fork()) < 0) 
     {    
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }

     else if (pid == 0) 
     { 
          int pd[2];

          if(pipe(pd) == -1)
          {
               perror("Error creating pipe!\n");
               exit(-1);
          }

          if ((pid = fork()) < 0) 
          {    
               printf("*** ERROR: forking child process failed\n");
               exit(1);
          }

          else if (pid == 0) 
          { 

               getTempv(&temp, &tempv, argv, 0, programCount[0]);

               close(1);
               dup(pd[1]);
               close(pd[0]);

               if (execv(temp, tempv) < 0) 
               {    
                    printf("*** ERROR: exec failed\n");
                    exit(1);
               }

               if(tempv)
                    delete[] tempv;

               close(pd[1]);

               exit(0);
          }

          else
          {     

               if(!interaction)                             
                    while (wait(0) != pid); 

               close(0);
               dup(pd[0]);
               close(pd[1]);

               getTempv(&temp, &tempv, argv, programCount[0]+1, number-programCount[0]-1);

               if (execv(temp, tempv) < 0) 
               {    
                    printf("*** ERROR: exec failed\n");
                    exit(1);
               }

               if(tempv)
                    delete[] tempv;

               close(pd[0]);
               exit(0);
          }
     }

     if(interaction)
     {
          backgroundProcess bp(pid, line);
          background.push_back(bp);
     }

     else                            
          while (wait(0) != pid); 

}


void performFourthType(char **argv, const bool& interaction, const int& number, char* line)
{
     pid_t pid;

     char* temp = NULL;
     char** tempv = NULL;

     if ((pid = fork()) < 0) 
     {    
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }

     else if (pid == 0) 
     { 
          int pd[2];
          if(pipe(pd) == -1)
          {
               perror("Error creating pipe!\n");
               exit(-1);
          }

          if ((pid = fork()) < 0) 
          {    
               printf("*** ERROR: forking child process failed\n");
               exit(1);
          }

          else if (pid == 0) 
          { 
               if(redirectionCount[0]<programCount[0])
               {
                    getTempv(&temp, &tempv, argv, 0, redirectionCount[0]);

                    getRedirection(argv, redirectionCount[0]); 

               }

               else if(redirectionCount[0]>programCount[0])
               {
                    getTempv(&temp, &tempv, argv, 0, programCount[0]);
               }

               close(1);
               dup(pd[1]);
               close(pd[0]);

               if (execv(temp, tempv) < 0) 
               {    
                    printf("*** ERROR: exec failed\n");
                    exit(1);
               }

               if(tempv)
                    delete[] tempv;

               close(pd[1]);

               exit(0);
          }

          else
          {                           
               while (wait(0) != pid); 

               close(0);
               dup(pd[0]);
               close(pd[1]);

               if(redirectionCount.size()==1)
               {
                    if(redirectionCount[0]>programCount[0])
                    {
                         getTempv(&temp, &tempv, argv, programCount[0]+1, redirectionCount[0]-programCount[0]-1);
                         getRedirection(argv, redirectionCount[0]);
                    }
                    else
                    {
                         getTempv(&temp, &tempv, argv, programCount[0]+1, number-programCount[0]-1);
                    }
               }
               else if(redirectionCount.size() == 2)
               {
                    getTempv(&temp, &tempv, argv, programCount[1]+1, redirectionCount[1]-programCount[0]-1);
                    getRedirection(argv, redirectionCount[1]);
               }

               if (execv(temp, tempv) < 0) 
               {    
                    printf("*** ERROR: exec failed\n");
                    exit(1);
               }

               if(tempv)
                    delete[] tempv;

               close(pd[0]);

               exit(0);
          }
     }

     if(interaction)
     {
          backgroundProcess bp(pid, line);
          background.push_back(bp);
     }

     else                            
          while (wait(0) != pid); 
}


void findPath()
{
     char *dup = strdup(getenv("PATH"));
     char *s = dup;
     char *p = NULL;
     do 
     {
         p = strchr(s, ':');
         if (p != NULL) {
             p[0] = 0;
         }
         path.push_back(s);
         s = p + 1;
     } while (p != NULL);

     path.push_back((char*)".");

     free(dup);
}


void reset(int& number, char** argv_, char* line)
{
     programCount.clear();
     redirectionCount.clear();
     for (int i = 0; i < number; ++i)
     {
          if(argv_[i]!=NULL)
               delete[] argv_[i];
     }
     number = 0;
     interaction = false;
     TYPE = -1;
     memset(line, 0, strlen(line));
}


void reset(int& number, char* line)
{
     programCount.clear();
     redirectionCount.clear();
     number = 0;
     interaction = false;
     TYPE = -1;
     memset(line, 0, strlen(line));
}


void checkIllInput(char **argv, bool& found)
{
     if(!programCount.empty())
     {
          if(!redirectionCount.empty())
          {
               for (int i = 0; i < redirectionCount.size(); ++i)
               {
                    if(argv[redirectionCount[i]][0] == '>' && redirectionCount[i] < programCount[0])
                    {
                         perror("Wrong command order! You can't have stdin before pipe!");
                         found = false;
                    }
               }
          }
     }
}

void setType()
{
     if(programCount.empty())
     {
          if(redirectionCount.empty())
          /* type 0 means no redirection nor pipe */
               TYPE = 0;
          else
          /* type 1 means no pipe but with redirection */
               TYPE = 1;
     }
     else
     {
          if(redirectionCount.empty())
          /* type 2 means with pipe but no redirection */
               TYPE = 2;
          /* type 3 means with pipe and redirection */
          else
               TYPE = 3;
     }
}

void findBinary(char **argv_, const int& number)
{
     char realPath[100];

     for (int i = 0; i < path.size(); ++i)
     {
          memset(realPath, 0, 100);
          sprintf(realPath, "%s%s%s", path[i],"/",argv_[0]);
          realPath[strlen(realPath)] = '\0';
          if(access(realPath, X_OK) == 0) /* find the command line */
          {
               memcpy(argv_[0], realPath, strlen(realPath));
               argv_[0][strlen(realPath)] = '\0';
               break;
          }
     }

     if(!programCount.empty())
     {
          const int& another = programCount[0]+1;
          for (int i = 0; i < path.size(); ++i)
          {
               memset(realPath, 0, 100);
               sprintf(realPath, "%s%s%s", path[i],"/",argv_[another]);
               if(access(realPath, X_OK) == 0) /* find the command line */
               {
                    memcpy(argv_[another], realPath, strlen(realPath));
                    argv_[another][strlen(realPath)] = '\0';
                    break;
               }
          }
     }
}


void increaseMomery(char **argv_, const int& number)
{
     char temp[100];
     for (int i = 0; i < number; ++i)
     {
          memset(temp, 0, 100);
          memcpy(temp, argv_[i], strlen(argv_[i]));
          argv_[i] = new char[100];
          memcpy(argv_[i], temp, strlen(temp));
          argv_[i][strlen(temp)] = '\0';
     }
}


void getTempv(char **temp, char ***tempv, char **argv, const int& start, const int& size)
{
     *tempv = new char*[size+1];
     for (int i = 0; i < size; ++i)
     {
          (*tempv)[i] = &argv[i+start][0];
     }
     (*tempv)[size] =  NULL;
     *temp = **tempv;
}


void getRedirection(char **argv, const int& index)
{
     int fd;
     if(argv[index][0] == '>')
     {
          fd = open(argv[index+1], O_WRONLY|O_CREAT, 0644);
          close(1); 
          dup(fd); 
          close(fd);
     }

     else if(argv[index][0] == '<')
     {
          fd = open(argv[index+1], O_RDONLY);
          close(0);
          dup(fd); 
          close(fd);
     }
}

void allTerminate()
{
     if(!background.empty())
     {
          for (int i = 0; i < background.size(); ++i)
          {
               if(kill(background[i].processID, SIGKILL) == -1)
               {
                    perror("Error killing process!\n");
                    exit(-1);
               }

               background.erase(background.begin()+i);
          }
     }
}

void printBackground()
{
     if(!background.empty())
     {
          int status;
          for (int i = 0; i < background.size(); ++i)
          {
               if(waitpid(background[i].processID,&status, WNOHANG) == -1)
               {
                    background.erase(background.begin()+i);
               }
               else
                    printf("%d  %s\n", background[i].processID, background[i].command);
          }
     }
}
