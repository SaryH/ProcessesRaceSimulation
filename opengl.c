#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <GL/glut.h>
#include "local.h"

struct message msg;
static char buffer[B_SIZ];
int WINDOW_HEIGHT = 1000 ;
int WINDOW_WIDTH = 1000 ;
int updateTime = 100; //microseconds
int a = 100;
int b = 200;
int x = 100;
int y = 100;

int n,myfifo;
static char buffer[B_SIZ];
char parentId[20];
int parentFifo;

typedef struct point {
	int x;
	int y;
} point;

point points[5];	    //A0 - A5
point displacement[2][5]; //xp yp team 1 and 2

int order[2];

int moves[2];

int change_x[2];// (x_end - x_start)/speed;
int change_y[2]; //(y_end - y_start)/speed;

// start & end = point[]
int x_start = 500;
int y_start = 500;
int x_end = 650;
int y_end = 200;


void setupScene(int clearColor[]) {
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    //glClearColor(250, 250, 250, 1.0);  //  Set the cleared screen colour to black.
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);  // This sets up the viewport so that the coordinates (0, 0) are at the top left of the window.
    
    // Set up the orthographic projection so that coordinates (0, 0) are in the top left.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -10, 10);
    
    // Back to the modelview so we can draw stuff.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen and depth buffer.
}

    
void initVariables(){
	//creating fifo for children to contact through
	sprintf(msg.fifo_name, "%d", getpid());
	if (mknod(msg.fifo_name, S_IFIFO | 0666, 0) < 0){
        perror("Private fifo creation");
        exit(-8);
    	}
    	myfifo=open(msg.fifo_name, O_RDONLY | O_NONBLOCK);
	
	points[0].x = 200;
	points[0].y = 500; 
	points[1].x = 500;
	points[1].y = 200; 
	points[2].x = 800;
	points[2].y = 500; 
	points[3].x = 700;
	points[3].y = 800; 
	points[4].x = 300;
	points[4].y = 800; 
    
    	for(int i=0; i<5; i++){
    		displacement[0][i].y = displacement[1][i].y =0;
    		displacement[0][i].x = displacement[1][i].x =0;
    	}
    	
        glPointSize(20);
}

void drawRaceGround(){
    glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);
    glVertex2d(450, 200);
    glVertex2d(150, 500);
    glVertex2d(250, 500);
    glVertex2d(550, 200);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);
    glVertex2d(450, 200);
    glVertex2d(750, 500);
    glVertex2d(850, 500);
    glVertex2d(550, 200);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);
    glVertex2d(750, 500);
    glVertex2d(650, 800);
    glVertex2d(750, 800);
    glVertex2d(850, 500);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);
    glVertex2d(750, 750);
    glVertex2d(250, 750);
    glVertex2d(250, 820);
    glVertex2d(750, 820);
    glEnd();
   
    glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);
    glVertex2d(150, 500);
    glVertex2d(250, 800);
    glVertex2d(350, 800);
    glVertex2d(250, 500);
    glEnd();
    
    for(int i=0; i<5; i++){
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(points[i].x, points[i].y);
    glEnd();
    }

}

void drawScene() {
    setupScene((int[]){250, 250, 250, 1});    
    drawRaceGround();  

    //move team1
    for(int i=0; i<5; i++){
	    glPushMatrix();
	    glTranslatef(displacement[0][i].x, displacement[0][i].y, 0);
	    glBegin(GL_POINTS);
	    glColor3ub(0, 255, 0);
	    glVertex2d(points[i].x, points[i].y);
	    glEnd();
	    glPopMatrix();
     }
     
      //move team2
    for(int i=0; i<5; i++){
	    glPushMatrix();
	    glTranslatef(displacement[1][i].x, displacement[1][i].y, 0);
	    glBegin(GL_POINTS);
	    glColor3ub(255, 0, 0);
	    glVertex2d(points[i].x, points[i].y);
	    glEnd();
	    glPopMatrix();
     }
     
	
/*

    //move rect
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);

    glVertex2d(b, a);
    glVertex2d(a, a);
    glVertex2d(a, b);
    glVertex2d(b, b);

    glEnd();
    glPopMatrix();

*/

    glutSwapBuffers();  // Send the scene to the screen.
    glFlush();
}

void update(int value) {
	
	//kill(getpid(),SIGUSR1);
	//reading from fifo for updates
	
	if(moves[0] == 0 || moves[1] == 0)
	if((n=read(myfifo,&buffer,B_SIZ))>0){

		//printf("\n\n\t\tOPENGL GOT THE DATA: %s",buffer);
		//fflush(stdout);

		char delim[] = " ";
		char* token = strtok(buffer, delim);
		int currentOrder = atoi(token);
		int team = (currentOrder > 4) ? 1 : 0;
		//printf("____________%d",team);
		currentOrder = currentOrder%5;
		token = strtok(0, delim);
		int timeNeeded =atoi(token);
		
		//printf("\ncurrentOrder = %d, timeNeeded = %d",currentOrder, timeNeeded);
		//fflush(stdout);
		
		x_start = points[currentOrder].x;	  // set start cordinates 
		y_start = points[currentOrder].y;
		x_end = points[((currentOrder) + 1)%5].x;  // set end coordinates at next point
		y_end = points[((currentOrder) + 1)%5].y;
		
		
		displacement[team][currentOrder].x = 0;
		displacement[team][currentOrder].y = 0;

		
		order[team] = currentOrder;
		moves[team] = timeNeeded*1000/updateTime;


		change_x[team] = (x_end - x_start)/moves[team];
		change_y[team] = (y_end - y_start)/moves[team];

	}
	
	
		//printf("_____disp of team %d, is %d",1, displacement[1][order[1]].x);
		
	for(int ofTeam=0; ofTeam<2; ofTeam++){
		if(moves[ofTeam] > 0){  
			moves[ofTeam]--;
			displacement[ofTeam][order[ofTeam]].x += change_x[ofTeam];
			displacement[ofTeam][order[ofTeam]].y += change_y[ofTeam];
			//printf("_____disp of team %d, is %d",ofTeam, displacement[ofTeam][order[ofTeam]].x);
			if(moves[ofTeam] == 0 && order[ofTeam] == 4){
			    	for(int i=0; i<5; i++){
		    			displacement[0][i].y = displacement[1][i].y =0;
		    			displacement[0][i].x = displacement[1][i].x =0;
		    		}
				 for(int i=0; i<2; i++){
				    	moves[i] = 0;
					change_x[i] = 0;
					change_y[i] = 0;
					order[i] = 0; 
				}
			}
		}
	}
	
/*
	if (x < 400 && y < 200) {
        x += speed1;
        y += speed1;
    }
    */
    
    glutPostRedisplay();  // Tell GLUT that the display has changed.
    glutTimerFunc(100, update, 0);  // Tell GLUT to call update again in 25 milliseconds.
}

int main(int argc, char * argv[]){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	initVariables();
	//initRendering();
	glutDisplayFunc(drawScene);
	//glutReshapeFunc(resizeWindow);
	//glutKeyboardFunc(myKeyboardFunc);
	//glutMouseFunc(myMouseFunc);
	glutTimerFunc(100, update, 0);
	glutMainLoop();
	close(myfifo);

	return 0;					// This line is never reached
}

