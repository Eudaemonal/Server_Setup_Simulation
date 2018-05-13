#include <iostream>
#include "simulation.hpp"
#include <map>
#include <random>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>



#define FONT GLUT_BITMAP_9_BY_15

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
static std::vector<float> y_value;
static std::vector<float> x_value;



void createHistogram(int len, std::vector<float> data){
    float x, y = 0.0, width = 2.0;
    int i;
    for(i=0; i<len; i++){
        x = (i * width) + i*1;
        createBox(x, y, width, data[i]);

		std::stringstream stream;
		stream << std::fixed << std::setprecision(1) << x_value[i];
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
    createHistogram(y_value.size(), y_value);
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
    glutBitmapCharacter(FONT, str.at(i));
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
	float mu = 1;
	float lambda = 3.5;


	const int nrolls=10000;  // number of experiments
	const int nstars=100;    // maximum number of stars to distribute
	const int nintervals=10;


	//std::map<int, int> at_dist;
	//std::map<int, int> st_dist;
	int p[nintervals]={};
	

	std::random_device rd;


	std::mt19937 gen(1);
	std::exponential_distribution<float> ed1(lambda);



	float st = 0;
	float at = 0;
	for(int n = 0; n < nrolls; ++n){
		// for(int i=0; i < 3; ++i){
		// 	st+=rd.exponential(mu);
		// }
		// at = 1/rd.exponential(lambda);



		float number;
		number = ed1(gen);
		

		std::cout << number << "\n";
		if (number<1.0) ++p[int(nintervals*number)];


		//at_dist[(int)at]++;
		//st_dist[(int)st]++;
	}

	std::cout << "exponential_distribution (" << lambda << "):\n";
	std::cout << std::fixed; std::cout.precision(1);

	for (int i=0; i<nintervals; ++i) {
		std::cout << float(i)/nintervals << "-" << float(i+1)/nintervals << ": ";
		std::cout << std::string(p[i]*nstars/nrolls,'*') << std::endl;
	}




	return 0;

}