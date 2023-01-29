#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "local.h"

//inputs
char order[5];
char openglPID[10];

int flag;
int privatefifo,publicfifo,fp,fp2,n,buf;
int speed,distance,timeneeded;

int nextPID, orderInTeam;
int openglPIDInt;
int openglFIFO;

int pid_array[10];
int team1[5];
int team2[5];

void printfifo(int);

//colors
void green(){
    printf("\033[1;32m");
    //fflush(stdout);
}

void red(){
    printf("\033[1;31m");
    //fflush(stdout);
}

void normal(){
    printf("\033[0;37m");
    //fflush(stdout);
}

//input is: 0->team, 1->order in team, 2->distance to move, 3->pid of opengl
int main(int argc, char * argv[]){

	//sleep(1);
	void catcher2();
	
	sigset(SIGUSR1,catcher2);//setting sigusr1 to the function catcher 2, which helps wait for all fifos to be created before we continue
	sigset(SIGUSR2,printfifo);
	
	//reading from children file
	FILE *f1;
	if((f1=fopen("children.txt","r"))==NULL)
		exit(10);
	for(int i=0;i<10;i++){
		if (fscanf(f1, "%d", &pid_array[i]) != 1)
			exit(1);
		//printf("\nMy Brothers are:%d",pids[i]);
		//fflush(stdout);
		//sleep(1);
	}
	fclose(f1);
	
	sprintf(order,"%s",argv[1]);
	sprintf(openglPID,"%s",argv[3]);
	
	if(strcmp(argv[0],"1")==0)
		green();
	else
		red();
	
	srand(getpid());
	speed=rand()%10+1;
	distance=atoi(argv[2]);
	timeneeded=distance/speed;
	
	//writing current data to openglfifo so it can draw
	kill(getpid(),SIGUSR2);//writes data to the opengl fifo
	
	//printing data to console
	printf("\n\n\t\tPID: %d (order:%s), %s meters at %d m/s, I need %d seconds....",getpid(),argv[1],argv[2],speed,timeneeded);
	fflush(stdout);
	
	//sleeping the timeneeded
	sleep(timeneeded);
	
	if(strcmp(argv[0],"1")==0 && getpid()==pid_array[4]){
		sleep(2);
		green();
		printf("\n\n\t\tTeam 1 finished the race!");
		fflush(stdout);
		
		FILE *pidfile=fopen("roundwin.txt","w");
		if(pidfile!=NULL){
			fprintf(pidfile,"%d\n",1);
			fclose(pidfile);
		}
		for(int i=9;i>=0;i--){
			if(pid_array[i]!=getpid())
				kill(pid_array[i],SIGKILL);
		}
		exit(0);
	}else if(strcmp(argv[0],"2")==0 && getpid()==pid_array[9]){
		sleep(2);
		red();
		printf("\n\n\t\tTeam 2 finished the race!");
		fflush(stdout);
		FILE *pidfile=fopen("roundwin.txt","w");
		if(pidfile!=NULL){
			fprintf(pidfile,"%d",2);
			fclose(pidfile);
		}
		for(int i=9;i>=0;i--){
			if(pid_array[i]!=getpid())
				kill(pid_array[i],SIGKILL);
		}
		exit(0);
	}
	
	orderInTeam=atoi(argv[1])+1;
	nextPID=pid_array[orderInTeam];
	
	kill(nextPID,SIGUSR1);
	
	return 0;
}

void catcher2(int i){
	if(orderInTeam<=4){
		char order[5];
		sprintf(order,"%d",orderInTeam);
		char opengl[10];
		sprintf(opengl,"%d",openglPIDInt);
		char * arg_list[]={"1",order,"10",opengl,NULL};
		execv("./child",arg_list);
		perror("execv team1");
		exit(-3);
	}
	else if(orderInTeam<=9){
		char order[5];
		sprintf(order,"%d",orderInTeam);
		char opengl[10];
		sprintf(opengl,"%d",openglPIDInt);
		char * arg_list[]={"2",order,"10",opengl,NULL};
		execv("./child",arg_list);
		perror("execv team2");
		exit(-5);
	}
}

void printfifo(int id){

	if((openglFIFO=open(openglPID, O_RDWR))==-1){
		perror("\n\n\t\tError opening opengl's FIFO Fromchild");
	}else{
		char currentData[B_SIZ];
		sprintf(currentData,"%s %d",order,timeneeded);
		//sleep(1);
		write(openglFIFO,currentData,B_SIZ);
		//sleep(1);
	}
	close(openglFIFO);
	//sleep(1);
	//fflush(stdout);
}

