//COSC 3P98 Assignment 3 - & Justin Gula (5991492) & Liam Yethon (6255384) 
//Code referenced: http://cosc.brocku.ca/Offerings/3P98/course/OpenGL/3P98Examples/OpenGLExamples/rotate_light.c
//Bonus Options Used: 14, 17, 18, 19, 21, our own: pause particles in mid air

#if !defined(Linux)
#include <windows.h>           //Not Linux must be windows
#endif
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <freeglut.h>
#include <FreeImage.h>
#include <gl/GL.h>
#include <glut.h>
#include <vector>
#include <random>
#include <iostream>
#include <time.h>
#include <MMSystem.h>

#define X 0
#define Y 1
#define Z 2

using std::vector;

struct glob {
    float angle[3];
    int axis;
    int local;
    int maxAge;
    int iteration;
};

struct glob global = { {0.0,0.0,0.0},Y,GL_FALSE, 1000, 0 };

typedef struct part {
    float posX, posY, posZ;
    float dirX, dirY, dirZ;
    float speed;
    float scale;
    float r, g, b;
    int age;
    int shape;
    bool isCamera;
} particle;

vector<particle> particlelist;
bool randomSpeed = false;
bool lock = false;
bool pauseMode = false;
bool wireMode = false;
bool nextCamMode = false;
int fireMode = 0;
float gravity = 0.05;

auto gen = std::mt19937{ std::random_device{}() };
auto dist = std::uniform_real_distribution<float>{ -0.6, 0.6 };
auto distY = std::uniform_real_distribution<float>{ 0.75 , 1.5 };
auto distSpeed = std::uniform_real_distribution<float>{ 0.75, 2.0 };
auto distColor = std::uniform_real_distribution<float>{ 0.0, 1.0 };
auto distShape = std::uniform_real_distribution<float>{ 0, 2 };

void myLightInit() {
    GLfloat ambient[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat local_view[] = { 0.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

    glEnable(GL_LIGHTING);   /* turns on lighting */
    glEnable(GL_LIGHT0);     /* turns on light 0  */
    /* glEnable(GL_NORMALIZE); not necessary here. */
}

void drawgroundhelper(float x1, float x2, float z1, float z2) {
    float pt[][3] = { {x1,2,z1},
                    {x1,-2,z1},
                    {x2,-2,z1},
                    {x2,2,z1},
                    {x1,2,z2},
                    {x1,-2,z2},
                    {x2,-2,z2},
                    {x2,2,z2} };

    int face[][4] = { {0,3,2,1},{3,7,6,2},{7,4,5,6},{4,0,1,5}, {0,4,7,3},{1,2,6,5} };

    float norm[][3] = { {0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0} };

    GLfloat mat_ambient[] = { 0.0215, 0.1745, 0.0215, 1.0 };
    GLfloat mat_diffuse[] = { 0.498, 0.415, 0.294, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 77.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glRotatef(global.angle[X], 1.0, 0.0, 0.0);
    glRotatef(global.angle[Y], 0.0, 1.0, 0.0);
    glRotatef(global.angle[Z], 0.0, 0.0, 1.0);

    int i;
    for (i = 0; i < 6; ++i) {
        glNormal3fv(norm[i]);
        glBegin(GL_POLYGON);
        glVertex3fv(pt[face[i][0]]);
        glVertex3fv(pt[face[i][1]]);
        glVertex3fv(pt[face[i][2]]);
        glVertex3fv(pt[face[i][3]]);
        glEnd();
    }
}

void drawground() {
    //drawgroundhelper(right, left, top, bottom)
    drawgroundhelper(50, -50, 50, -20);
    drawgroundhelper(17.5, -50, -20, -50);
    drawgroundhelper(50, 32.5, -20, -50);
    drawgroundhelper(32.5, 17.5, -40, -50);
}

void drawfountain() {
    int pt[][3]{ {1,40,1}, {5,0,5}, {-5,0,5}, {-2,40,2},
                   {2,40,-2}, {5,0,-5}, {-5,0,-5}, {-1,40,-1} };

    int face[][4] = { {0,3,2,1},{3,7,6,2},{7,4,5,6},{4,0,1,5}, {0,4,7,3},{1,2,6,5} };

    float norm[][3] = { {0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0} };

    GLfloat mat_ambient[] = { 0.0215, 0.1745, 0.0215, 1.0 };
    GLfloat mat_diffuse[] = { 0.654, 0.580, 0.454, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 77.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glRotatef(global.angle[X], 1.0, 0.0, 0.0);
    glRotatef(global.angle[Y], 0.0, 1.0, 0.0);
    glRotatef(global.angle[Z], 0.0, 0.0, 1.0);

    int i;
    for (i = 0; i < 6; ++i) {
        glNormal3fv(norm[i]);
        glBegin(GL_POLYGON);
        glVertex3iv(pt[face[i][0]]);
        glVertex3iv(pt[face[i][1]]);
        glVertex3iv(pt[face[i][2]]);
        glVertex3iv(pt[face[i][3]]);
        glEnd();
    }
}

void drawfountaintip() {
    int pt[][3]{ {0,45,0}, {1,40,1}, {-2,40,2}, {0,45,0},
                   {0,45,0}, {2,40,-2}, {-1,40,-1}, {0,45,0} };

    int face[][4] = { {0,3,2,1},{3,7,6,2},{7,4,5,6},{4,0,1,5}, {0,4,7,3},{1,2,6,5} };

    float norm[][3] = { {0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0} };

    GLfloat mat_ambient[] = { 0.0215, 0.1745, 0.0215, 1.0 };
    GLfloat mat_diffuse[] = { 0.654, 0.580, 0.454, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 77.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glRotatef(global.angle[X], 1.0, 0.0, 0.0);
    glRotatef(global.angle[Y], 0.0, 1.0, 0.0);
    glRotatef(global.angle[Z], 0.0, 0.0, 1.0);

    int i;
    for (i = 0; i < 6; ++i) {
        glNormal3fv(norm[i]);
        glBegin(GL_POLYGON);
        glVertex3iv(pt[face[i][0]]);
        glVertex3iv(pt[face[i][1]]);
        glVertex3iv(pt[face[i][2]]);
        glVertex3iv(pt[face[i][3]]);
        glEnd();
    }
}

void drawcubeparticle(int index) {
    
    //Center of object is posX, posY, posZ
    //scale + posX

    particle current = particlelist[index];

    float posScale = current.scale;
    float negScale = current.scale * -1; 

    float posX = current.posX;
    float posY = current.posY;
    float posZ = current.posZ;

    float pt[][3]{ {posScale + posX, posScale + posY, posScale + posZ}, 
                 {posScale + posX, negScale + posY, posScale + posZ}, 
                 {negScale + posX, negScale + posY, posScale + posZ}, 
                 {negScale + posX, posScale + posY, posScale + posZ},
                 {posScale + posX, posScale + posY, negScale + posZ}, 
                 {posScale + posX, negScale + posY, negScale + posZ}, 
                 {negScale + posX, negScale + posY, negScale + posZ},
                 {negScale + posX, posScale + posY, negScale + posZ} 
              };


    int face[][4] = { {0,3,2,1},{3,7,6,2},{7,4,5,6},{4,0,1,5}, {0,4,7,3},{1,2,6,5} };

    float norm[][3] = { {0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0} };

    GLfloat mat_ambient[] = { 0.0215, 0.1745, 0.0215, 1.0 };
    GLfloat mat_diffuse[] = { current.r, current.g, current.b, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 77.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    int i;
    for (i = 0; i < 6; ++i) {
        glNormal3fv(norm[i]);
        glBegin(GL_POLYGON);
        glVertex3fv(pt[face[i][0]]);
        glVertex3fv(pt[face[i][1]]);
        glVertex3fv(pt[face[i][2]]);
        glVertex3fv(pt[face[i][3]]);
        glEnd();
    }
}

void drawtriangleparticle(int index) {
    //Center of object is posX, posY, posZ
    //scale + posX

    particle current = particlelist[index];

    float posScale = current.scale * 1.25;
    float negScale = current.scale * -1.25;

    float posX = current.posX;
    float posY = current.posY;
    float posZ = current.posZ;

    float pt[][3]{ {posX, posScale + posY, posZ}, 
                 {posScale + posX, posY , posScale + posZ}, 
                 {negScale + posX, posY, posScale + posZ}, 
                 {posX, posScale + posY, posZ},
                 {posX, posScale + posY, posZ}, 
                 {posScale + posX, posY, negScale + posZ}, 
                 {negScale + posX, posY, negScale + posZ}, 
                 {posX, posScale + posY, posZ}};


    int face[][4] = { {0,3,2,1},{3,7,6,2},{7,4,5,6},{4,0,1,5}, {0,4,7,3},{1,2,6,5} };

    float norm[][3] = { {0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0} };

    GLfloat mat_ambient[] = { 0.0215, 0.1745, 0.0215, 1.0 };
    GLfloat mat_diffuse[] = { current.r, current.g, current.b, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 77.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    int i;
    for (i = 0; i < 6; ++i) {
        glNormal3fv(norm[i]);
        glBegin(GL_POLYGON);
        glVertex3fv(pt[face[i][0]]);
        glVertex3fv(pt[face[i][1]]);
        glVertex3fv(pt[face[i][2]]);
        glVertex3fv(pt[face[i][3]]);
        glEnd();
    }
}

void spawn(int amt, bool isCamera) {
    for (int i = 0; i < amt; i++) {
        particle input;
        input.posX = 0.0;
        input.posY = 50.0;
        input.posZ = 0.0;

        input.dirX = dist(gen);
        input.dirY = distY(gen);
        input.dirZ = dist(gen);

        if (randomSpeed) {
            input.speed = distSpeed(gen);
        }
        else {
            input.speed = 0.75;
        }

        input.scale = distSpeed(gen);
        input.r = distColor(gen);
        input.g = distColor(gen);
        input.b = distColor(gen);
        input.age = 0;
        input.shape = (int)distShape(gen);
        input.isCamera = nextCamMode;
        if (nextCamMode) {
            nextCamMode = false;
        }
        particlelist.push_back(input);
    }
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (global.iteration % 25 == 0 && particlelist.size() < 200 && fireMode == 0 && !pauseMode) {
        spawn(1, nextCamMode);
        PlaySound(TEXT("pop.wav"), NULL, SND_ASYNC | SND_FILENAME);
    }

    drawground();
    drawfountain();
    drawfountaintip();
    
    for (int i = 0; i < particlelist.size(); i++) {
        if (!pauseMode) {

            particle& current = particlelist.at(i);

            //Direction
                //if direction Y is 0, apply -10% to remaining to signify friction
                //else update direction Y with gravity
            if (current.dirY == 0) {
                if (current.posX > 50 || current.posX < -50) {
                    current.dirY = current.dirY - gravity;
                }
                else if (current.posZ > 50 || current.posZ < -50) {
                    current.dirY = current.dirY - gravity;
                }
                else if (current.posX < 32.5 && current.posX > 17.5) {
                    if (current.posZ > -40 && current.posZ < -20) {
                        current.dirY = current.dirY - gravity;
                    }
                    else {
                        current.dirX = current.dirX * 0.9;
                        current.dirZ = current.dirZ * 0.9;
                        if (abs(current.dirX) < 0.05) {
                            current.dirX = 0;
                        }
                        if (abs(current.dirZ) < 0.05) {
                            current.dirZ = 0;
                        }
                    }
                }
                else {
                    current.dirX = current.dirX * 0.9;
                    current.dirZ = current.dirZ * 0.9;
                    if (abs(current.dirX) < 0.05) {
                        current.dirX = 0;
                    }
                    if (abs(current.dirZ) < 0.05) {
                        current.dirZ = 0;
                    }
                }
            }
            else {
                current.dirY = current.dirY - gravity;
            }

            //Rotation

            //Position
            current.posX = current.posX + (current.dirX * current.speed);
            current.posY = current.posY + (current.dirY * current.speed);
            current.posZ = current.posZ + (current.dirZ * current.speed);

            //Is it on square 1?
            if (current.posX >= -49 && current.posX <= 49) {
                if (current.posZ <= 49 && current.posZ >= -20) {
                    if (current.posY - current.scale <= 2) {
                        current.posY = 2 + current.scale;
                        current.dirY *= -0.3;

                        if (abs(current.dirY) < 0.25) {
                            current.dirY = 0;
                        }
                    }
                }
            }

            //Is it on square 2?
            if (current.posX >= -49 && current.posX <= 17.5) {
                if (current.posZ <= -20 && current.posZ >= -49) {
                    if (current.posY - current.scale <= 2) {
                        current.posY = 2 + current.scale;
                        current.dirY *= -0.3;

                        if (abs(current.dirY) < 0.25) {
                            current.dirY = 0;
                        }
                    }
                }
            }

            //Is it on square 3?
            if (current.posX >= 32.5 && current.posX <= 49) {
                if (current.posZ <= -20 && current.posZ >= -49) {
                    if (current.posY - current.scale <= 2) {
                        current.posY = 2 + current.scale;
                        current.dirY *= -0.3;

                        if (abs(current.dirY) < 0.25) {
                            current.dirY = 0;
                        }
                    }
                }
            }

            //Is it on square 4?
            if (current.posX >= 17.5 && current.posX <= 32.5) {
                if (current.posZ <= -40 && current.posZ >= -49) {
                    if (current.posY - current.scale <= 2) {
                        current.posY = 2 + current.scale;
                        current.dirY *= -0.3;

                        if (abs(current.dirY) < 0.25) {
                            current.dirY = 0;
                        }
                    }
                }
            }


            //Check age > maxAge
            if (current.age > global.maxAge) {
                if (current.isCamera) {
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    gluLookAt(75, 50, 75, 0, 0, 0, 0, 1, 0);
                }
                particlelist.erase(particlelist.begin() + i);
            }
            else if (current.posY < -250) {
                if (current.isCamera) {
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    gluLookAt(75, 50, 75, 0, 0, 0, 0, 1, 0);
                }
                particlelist.erase(particlelist.begin() + i);
            }
            else if (current.dirX == 0 && current.dirY == 0 && current.dirZ == 0) {
                current.age += 5;
                if (current.shape == 0) {
                    drawcubeparticle(i);
                }
                else if (current.shape == 1) {
                    drawtriangleparticle(i);
                }
                if (current.isCamera) {
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    gluLookAt(current.posX, current.posY, current.posZ, 0, 50, 0, 0, 1, 0);
                }
            }
            else {
                current.age++;
                if (current.shape == 0) {
                    drawcubeparticle(i);
                }
                else if (current.shape == 1) {
                    drawtriangleparticle(i);
                }

                if (current.isCamera) {
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    gluLookAt(current.posX, current.posY, current.posZ, 0, 50, 0, 0, 1, 0);
                }
            }
        }
        else {
            particle& current = particlelist.at(i);

            if (current.shape == 0) {
                drawcubeparticle(i);
            }
            else if (current.shape == 1) {
                drawtriangleparticle(i);
            }

            if (current.isCamera) {
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(current.posX, current.posY, current.posZ, 0, 50, 0, 0, 1, 0);
            }
        }
    }
    

    //For each object in particlelist;
        //compute new position
            //Direction: updated according to rules of environment
            //Rotation: updated based on new direction, or random spinning
            //Position: Position + Direction * Speed
            //check if age > maxAge
            //Render particle with new values
    global.iteration++;
    glutSwapBuffers();
}

void userintro() {
    printf("COSC 3P98 Assignment 3 - Liam Yethon + Justin Gula");
    printf("\nLeft Mouse - Rotate left");
    printf("\nRight Mouse - Rotate right");
    printf("\nx, y, z - Rotate around x, y, or z-axis");
    printf("\nl - Toggle local viewing for lighting");
    printf("\ns - Toggle between fixed or random particle speed");
    printf("\nr - Pause rotation");
    printf("\nt - Toggle between constant, stream, and single shot fire mode");
    printf("\nf - Fire particles");
    printf("\np - Pause movement of particles");
    printf("\nm - Toggle default and wireframe");
    printf("\nk - Next particle is camera");
    printf("\nq - Quit");
}

void keyboard(unsigned char key, int x, int y) {

    switch (key) {
    case 'x':
    case 'X':
        global.axis = X;
        printf("\nNow rotating around the X-axis.");
        break;

    case 'y':
    case 'Y':
        global.axis = Y;
        printf("\nNow rotating around the Y-axis.");
        break;

    case 'z':
    case 'Z':
        global.axis = Z;
        printf("\nNow rotating around the Z-axis.");
        break;

    case 'l':
    case 'L':
        global.local = !global.local;
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, global.local);
        break;

    case 's':
    case 'S':
        randomSpeed = !randomSpeed;
        if (randomSpeed) {
            printf("\nNow randomizing particle speed.");
        }
        else {
            printf("\nNo longer randomizing particle speed.");
        }
        break;

    case 'r':
    case 'R':
        global.angle[X] = 0.0;
        global.angle[Y] = 0.0;
        global.angle[Z] = 0.0;
        particlelist.clear();
        printf("\nClearing particle list.");
        break;

    case 'f':
    case 'F':
        if (fireMode == 1) {
            glutSetKeyRepeat(true);
            if (global.iteration % 3 == 0 && particlelist.size() < 200) {
                spawn(1, nextCamMode);
                PlaySound(TEXT("pop.wav"), NULL, SND_ASYNC | SND_FILENAME);
            }
        }
        else if (fireMode == 2) {
            glutSetKeyRepeat(false);
            spawn(1, nextCamMode);
            PlaySound(TEXT("pop.wav"), NULL, SND_ASYNC | SND_FILENAME);
        }
        break;

    case 't':
    case 'T':
        fireMode++;
        if (fireMode > 2) {
            fireMode = 0;
        }
        if (fireMode == 0) {
            printf("\nConstant mode.");
        }
        else if (fireMode == 1) {
            printf("\nStream mode. Press and hold F to fire.");
        }
        else {
            printf("\nSingle shot mode. Press F to fire.");
        }
        break;

    case 'm':
    case 'M':
        wireMode = !wireMode;
        if (wireMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            printf("\nNow in wireframe view mode.");
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            printf("\nNow in default view mode.");
        }
        break;

    case 'k':
    case 'K':
        nextCamMode = true;
        printf("\nNext particle will be camera.");
        break;

    case 'p':
    case 'P':
        pauseMode = !pauseMode;
        if (pauseMode) {
            printf("\nNow paused.");
        }
        else {
            printf("\nNow unpaused.");
        }
        break;

    case 0x1B:
    case 'q':
    case 'Q':
        exit(0);
        break;
    }
}

void mouse(int btn, int state, int x, int y) {

    if (state == GLUT_DOWN) {
        if (btn == GLUT_LEFT_BUTTON) {
            global.angle[global.axis] = 0.05;
        }
        else if (btn == GLUT_RIGHT_BUTTON) {
            global.angle[global.axis] = -0.05;
        }
    }
}

int main(int argc, char** argv) {

    userintro();

    srand((unsigned)time(0));

    glutInit(&argc, argv);
    glutInitWindowSize(750, 750);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("COSC 3P98 Assignment 3");
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(draw);
    glutIdleFunc(draw);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(100, 750 / 750, 1, 300);

    glMatrixMode(GL_MODELVIEW);
    gluLookAt(75, 50, 75, 0, 0, 0, 0, 1, 0);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    myLightInit();

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    global.angle[global.axis] = 0.05;

    glutMainLoop();
}