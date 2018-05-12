#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <sstream>
#include "simulation.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


/*
Useage: 
make
./design

*/

const int font=(int)GLUT_BITMAP_9_BY_15;

#define X_LIMIT 50.0      // limit
#define Y_LIMIT 20.0
#define X_CENTER (X_LIMIT - X_LIMIT - 1)
#define Y_CENTER (Y_LIMIT - Y_LIMIT - 1)
// prototype
void setup();             // initialization of the program
void display();           // drawing method
void createCoordinate();  // certasian coordinate
void draw();              // draw the object
void createBox(float, float, float, float);
void createHistogram(int len, std::vector<float> data);
void renderBitmapString(float x,float y, std::string str);

// all mean response times
static std::vector<float> mrts;
// all test cases of Tc
static std::vector<float> tcv;

float run(int seqnum, float d_time){
	// setup containers for arguments
	std::string mode;

	int n_server;
	float setup_time;
	float delayedoff_time;
	float time_end;
	bool reproducible = false;

	std::vector<float> arrival;
	std::vector<float> service;

	// read arguments from files
	mode = "random";

	n_server = 5;
	setup_time = 5;
	delayedoff_time = d_time;
	time_end = 100;

	arrival.push_back(0.35);
	service.push_back(1);
	
	// check arguemnt value
	debug_cout( "Mode: " << mode << "\n");
	debug_cout( "N server: " << n_server << "\n");
	debug_cout( "setup: " << setup_time << "\n");
	debug_cout( "delayed off: " << delayedoff_time << "\n");
	debug_cout( "arrival: "<< arrival);
	debug_cout( "service: " << service);

	// run simulation function, results store in finished_jobs
	std::vector<Job* > finished_jobs;
	finished_jobs = simulate(mode, arrival, service, n_server, setup_time,
	       			delayedoff_time, time_end, reproducible);

	float sum = 0;
	for(int i = 0; i < finished_jobs.size(); ++i){
		sum += (finished_jobs[i]->get_departure() - finished_jobs[i]->get_arrival());
	}
	return sum / (float)finished_jobs.size();
}

void createHistogram(int len, std::vector<float> data){
    float x, y = 0.0, width = 2.0;
    int i;
    for(i=0; i<len; i++){
        x = (i * width) + i*1;
        createBox(x, y, width, data[i]);

		std::stringstream stream;
		stream << std::fixed << std::setprecision(1) << tcv[i];
		std::string sn = stream.str();

        renderBitmapString(x, y, sn);
    }
    renderBitmapString(x+3, 0.0, "Tc");
    renderBitmapString(0.7, data[0]+5, "MRT");
}

void setup(){
    glClearColor(1.0, 1.0, 1.0, 1.0);
    gluOrtho2D(X_CENTER, X_LIMIT, Y_CENTER, Y_LIMIT); // -x1, x2, -y1, y2
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.3, 0.3, 0.3);

    draw();
    createCoordinate();

    glFlush();
}

void draw(){
    createHistogram(mrts.size(), mrts);
}



void createBox(float x, float y, float width, float height){
    glBegin(GL_POLYGON);
        glVertex2f(x, y);
        glVertex2f(x, y+height);
        glVertex2f(x+width, y+height);
        glVertex2f(x+width, y);
    glEnd();
}

void createCoordinate(){
    glBegin(GL_LINES);
        // horizontal lines
        glVertex2f(-X_LIMIT, 0.0);
        glVertex2f(X_LIMIT, 0.0);

        // vertical lines
        glVertex2f(0.0, -Y_LIMIT);
        glVertex2f(0.0, Y_LIMIT);
    glEnd();
}


 
void renderBitmapString(float x,float y, std::string str) {  
  char *c;

  glRasterPos2f(x, y-0.8);
  for(int i=0; i<str.length(); ++i){
    glutBitmapCharacter((void *)font, str.at(i));
  }

  
}


void draw_histogram(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(200, 100);
    glutInitWindowSize(int(X_LIMIT*20), int(Y_LIMIT*20));
    glutCreateWindow("Histogram");
    glutDisplayFunc(display);

    setup();
    glutMainLoop();
}


int main(int argc, char *argv[]){
	// each Tc value will run n_test times
	int n_test = 500;
	// determines cases of Tc
	int n_case = 16;
	float mrt;

	for(int i=0; i < n_case; ++i){
		tcv.push_back(0.1 * pow(2, i));
	}

	for(int i = 0; i < tcv.size(); ++i){
		mrt = 0;
		for(int j = 0; j < n_test; ++j){
			mrt += run(0, tcv[i]);
		}
		mrt = mrt/n_test;
		std::cout << "Tc: " << tcv[i] << " mean mrt: " << mrt << "\n";

		mrts.push_back(mrt);
	}

	draw_histogram(argc, argv);
}

