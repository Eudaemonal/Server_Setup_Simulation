#include <iostream>
#include "simulation.hpp"
#include <map>
#include <random>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>



#define FONT GLUT_BITMAP_9_BY_15

#define X_LIMIT 64.0      // limit
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


template <typename T>
T get_mean(std::vector<T> v){
	T m;
	T sum;
	for(auto it = v.cbegin(); it!=v.cend(); ++it){
		sum += *it;
	}
	m = sum / v.size();
	return m;
}

template <typename T>
T get_variance(std::vector<T> v){
	T m = get_mean(v);
	T tmp = 0;
	for(auto it = v.cbegin(); it!=v.cend(); ++it){
		tmp += pow(*it - m, 2);
	}
	return tmp / (v.size()-1);
}

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


void print_help(){
	std::cout << "Please specify an argument to verify\n";
	std::cout << "Useage: $ ./verify arrival\n";
	std::cout << "        $ ./verify service\n";
	exit(0);

}

int main(int argc, char *argv[]){
	float lambda = 0.35;
	float mu = 1;

	const int nrolls=10000;  // number of experiments
	const int nstars=100;    // maximum number of stars to distribute
	int resolution;
	int nintervals;

	bool aors= true;

	if(argc!=2)
		print_help();

	std::string type = argv[1];
	if(type=="arrival"){
		aors= true;
	}
	else if(type=="service"){
		aors= false;
	}
	else{
		print_help();
	}

	// arrival distribution
	if(aors==true){
		resolution=10;
		nintervals=20;
	}else{ // service distribution
		resolution=2;
		nintervals=20;
	}

	int p[nintervals]={};
	y_value.clear();
	x_value.clear();
	

	std::random_device rd;
	std::mt19937 gen(rd());
	
	std::exponential_distribution<float> d_arrival(lambda);
	std::exponential_distribution<float> d_service(mu);


	float st = 0;
	float at = 0;
	float number;
	std::vector<float> sequence; 
	for(int n = 0; n < nrolls; ++n){
		st = 0;
		for(int i=0; i < 3; ++i){
			st += d_service(gen);
		}
		at = 1/d_arrival(gen);


		if(aors==true){
			number = at;
		}
		else{
			number = st;
		}

		sequence.push_back(number);
		//std::cout << number << "\n";
		if (number< nintervals/resolution) ++p[int(resolution*number)];

	}

	if(aors==true)
		std::cout << "Arrival time probability distribution\n";
	else
		std::cout << "Service time probability distribution\n";
	std::cout << std::fixed; std::cout.precision(1);

	for (int i=0; i<nintervals; ++i) {
		std::cout << float(i)/resolution << "-" << float(i+1)/resolution << ": ";
		std::cout << std::string(p[i]*nstars/nrolls,'*') << "\n";

		x_value.push_back(float(i)/resolution);
		y_value.push_back(float(p[i])*nstars/nrolls);
	}


	std::cout << std::fixed; std::cout.precision(5);
	
	std::cout << "mean:     " << get_mean<float>(sequence) << "\n";
	std::cout << "variance: " << get_variance<float>(sequence) << "\n";

	if(aors==true){
		std::cout << "expected mean:     " << (1/lambda) << "\n";
		std::cout << "expected variance: " << (1/pow(lambda, 2)) << "\n";
	}
	else{
		std::cout << "expected mean:     " << 3*(1/mu) << "\n";
		std::cout << "expected variance: " << 3*(1/pow(mu,2)) << "\n";
	}
	

	draw_histogram(argc, argv);

	return 0;

}