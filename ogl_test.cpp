#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <iostream>

/* Compile: g++ -framework OpenGL GLUT ogl_test.cpp -o ogl_test
 
g++ ogl_test.cpp -o ogl_test -lGLU -lGL -lglut 
 * */

using namespace std;

#define LIMIT 20.0      // limit
#define CENTER (LIMIT - LIMIT - 1)


// prototype
void setup();             // initialization of the program
void display();           // drawing method
void createCoordinate();  // certasian coordinate
void draw();              // draw the object
void createBox(float, float, float, float);
void createHistogram(int, float*);

void createHistogram(int len, float *data){
    float x, y = 1.0, width = 2.0;
    int i;
    for(i=0; i<len; i++){
        x = (i * width) + i + 1;
        createBox(x, y, width, data[i]);
    }
}


void setup(){
    glClearColor(1.0, 1.0, 1.0, 1.0);
    gluOrtho2D(CENTER, LIMIT, CENTER, LIMIT); // -x1, x2, -y1, y2
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 0.0, 0.0);

    draw();
    createCoordinate();

    glFlush();
}


void draw(){
    float value[] = {10.0, 15.0, 13.0, 9.0, 7.0, 10.};
    int len = 6;
    createHistogram(len, value);


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
        glVertex2f(-LIMIT, 0.0);
        glVertex2f(LIMIT, 0.0);

        // vertical lines
        glVertex2f(0.0, -LIMIT);
        glVertex2f(0.0, LIMIT);
    glEnd();
}

int main(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(200, 100);
    glutInitWindowSize(400, 300);
    glutCreateWindow("Hello World");
    glutDisplayFunc(display);

    setup();
    glutMainLoop();



    return 0;

}

