
#define _XOPEN_SOURCE 500
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"

//global variables
pid_t pid, pid_array[10],openglPID;//pid and pid array to store child pids
char *pid_file="children.txt";
int team1[5];//store pids of team1
int team2[5];//store pids of team2
int counter=0;//number of children
int team1_score,team2_score;
char myId[20];

struct message msg;
int n,myfifo;
static char buffer[B_SIZ];

//colors:
void green(){
    printf("\033[1;32m");
    fflush(stdout);
}

void red(){
    printf("\033[1;31m");
     fflush(stdout);
}

void purple(){
    printf(" \033[0;35m");
    fflush(stdout);
}

void normal(){
    printf("\033[0;37m");
    fflush(stdout);
}

void yellow(){
    printf("\033[1;33m");
    fflush(stdout);
}

//catcher function that will be used as the user signal sigusr1 to exec the children with the correct args
void catcher(int);
void openGLCatcher(int);

int main(int argc, char *argv[]){

	int rounds;
	if(argc!=2){
		printf("\n\n\t\tDefault of 5 rounds is selected");
		rounds=5;
	}else{
		rounds=atoi(argv[1]);
	}
	
	sigset(SIGUSR1,catcher);//setting the signal SIGUSR1 to use our catcher function
	
	for(int round=0;round<rounds;round++){
	
		openglPID=fork();
		switch(openglPID){
				case -1://failed to create opengl id
					exit(-1);
					break;
					
				case 0://currently in child
					execv("./opengl",NULL);
					break;
				default:
					sleep(2);//wait a bit for the opengl to run
					break;
		}
		
		purple();
		printf("\n\n\t\t-----------------------------------------------------------------");
		printf("\n\n\t\tWelcome to round #%d:",round+1);
		printf("\n\n\t\t-----------------------------------------------------------------");
		fflush(stdout);
		
		normal();
		counter=0;
		FILE *f=fopen("roundwin.txt","w");
		fclose(f);
		
		for(int i=0;i<10;i++){
			
			pid=fork();//create child
			switch(pid){
				case -1://failed to create child
					exit(-1);
					break;
					
				case 0://currently in child
					while(1){//wait until the SIGUSR1 signal is received
						sigpause(SIGUSR1);
					}
					exit(-2);
					break;
					
				default:
					pid_array[counter]=pid;
					counter++;//successful child creation
					//printf("\n\n\t\tI am the parent => PID = %d, child ID = %d", getpid(), pid);
					//fflush(stdout);
					if(counter==10){
						green();
						for(int j=0;j<5;j++){//storing team1
							team1[j]=pid_array[j];
							printf("\n\n\t\tID %d is in team 1",team1[j]);
						}
						red();
						for(int j=5;j<10;j++){//storing team2
							team2[j-5]=pid_array[j];
							printf("\n\n\t\tID %d is in team 2",team2[j-5]);
						}
						normal();
					}
					break;
			}
		}
		
		//printing pids of children to a local text file so that children created can read it and know their siblings
		FILE *pidfile=fopen(pid_file,"w");
		if(pidfile!=NULL){
			for(int i=0;i<10;i++){
				fprintf(pidfile,"%d\n",pid_array[i]);
			}
			fclose(pidfile);
		}
		
		yellow();
		printf("\n\n\t\tRACE STARTS!!");
		fflush(stdout);
		
		kill(pid_array[0],SIGUSR1);
		kill(pid_array[5],SIGUSR1);
		
		int wait=1;
		int teamwon=0;
		
		while(wait){
			sleep(1);
			FILE *f1;
			if((f1=fopen("roundwin.txt","r"))==NULL)
				continue;
			if (fscanf(f1, "%d", &teamwon) != 1)
				continue;
			if(teamwon==1){
				yellow();
				printf("\n\n\t\tTEAM 1 WON THE ROUND");
				team1_score++;
				fflush(stdout);
				wait=0;
			}else if(teamwon==2){
				yellow();
				printf("\n\n\t\tTEAM 2 WON THE ROUND");
				team2_score++;
				fflush(stdout);
				wait=0;
			}
			fclose(f1);
		}
		
		purple();
		printf("\n\n\t\tCurrent score is:\n\n\t\tTeam1:\t%d\n\n\t\tTeam2:\t%d",team1_score,team2_score);
		fflush(stdout);
		char removefifo[B_SIZ];
		sprintf(removefifo,"%d",openglPID);
		sleep(3);
		kill(openglPID,SIGKILL);
		remove(removefifo);
		remove(pid_file);
		remove("roundwin.txt");
		sleep(1);
		}
		
	return 0;
}

void catcher(int id){
	//depending on the position, team, and distance to be moved, the arguments are passed to the child 
	//children are also executed using the execv command
	if(counter<=4){
		char order[5];
		sprintf(order,"%d",counter);
		char opengl[10];
		sprintf(opengl,"%d",openglPID);
		char * arg_list[]={"1",order,"10",opengl,NULL};
		execv("./child",arg_list);
		perror("execv team1");
		exit(-3);
	}
	else if(counter<=9){
		char order[5];
		sprintf(order,"%d",counter);
		char opengl[10];
		sprintf(opengl,"%d",openglPID);
		char * arg_list[]={"2",order,"10",opengl,NULL};
		execv("./child",arg_list);
		perror("execv team2");
		exit(-5);
	}

}

