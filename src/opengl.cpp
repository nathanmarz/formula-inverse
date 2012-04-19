#include <GL/gl.h>
#include <glut.h>
#include <iostream>
#include <math.h>
#include <xsupport.h>
#include <Level.h>
#include <SDL.h>

using namespace std;

static int year = 0, day = 0, counter =0;
static float speed = 0;

#define PI 3.14159
#define HEIGHT 128
#define WIDTH 128

static GLubyte checkImage[HEIGHT][WIDTH][4];
static GLuint texName;

bool viewFlag = false;
int viewAngle = 0;
GLfloat viewPos[3] = {10,0,0};


static void LoadPPM(char *Name) 
{
	
	Canvas NewCanvas;
	
	if (!LoadCanvas(Name,&NewCanvas)) 
	{
		cout<<"Load failed!"<<endl;
		return;
	}

    for(int i=0;i<WIDTH&&i<NewCanvas.Width;i++) {
        for(int j=0;j<HEIGHT&&j<NewCanvas.Height;j++) {
            unsigned long p = PIXEL(&NewCanvas,i,j);
            checkImage[i][j][0] = GET_RED(p);
            checkImage[i][j][1] = GET_GREEN(p);
            checkImage[i][j][2] = GET_BLUE(p);
            checkImage[i][j][3] = GET_ALPHA(p);
        }
    }
		
	delete NewCanvas.Pixels;
}



void makeCheckImage() {
    LoadPPM("smallalps.ppm");
}

void crossProduct(float *c,float a[3], float b[3])  //finds the cross product of two vectors
{  
  c[0]=a[1]*b[2] - b[1]*a[2];
  c[1]=a[2]*b[0] - b[2]*a[0];
  c[2]=a[0]*b[1] - b[0]*a[1];
}


void myLook() {
    glRotatef(viewAngle,0,1,0);
    glTranslatef(viewPos[0],viewPos[1],viewPos[2]);
}

RPiece *piece1, *piece2, *piece3;

RTrack *track;

void init() {
    GLfloat mat_specular[] = { .5,.5,.5,.3 };
    GLfloat mat_shininess[] = {100.0};
    GLfloat mat_ambient[] = {.7,.2,.3,.5 };
 
    GLfloat white_light[] = {1.0,1.0,1.0,1.0 };
    GLfloat lmodel_ambient[] = {0.5,0.5,0.5,.5 };
    
    glClearColor(0.0,0.0,0.0,.5);
    glShadeModel(GL_SMOOTH);
    
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT,white_light);
  //  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lmodel_ambient);
    
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);

    RPieceShell rps1, rps2;
    ///*
    
    
    rps1.push_back(REdgeShell(Vector4(15,0,10),Vector4(10,-2,10)));
    rps1.push_back(REdgeShell(Vector4(15,-2,5),Vector4(10,-2,5)));
    vector<Vector4> ps;
    ps.push_back(Vector4(15,-0,0));
    ps.push_back(Vector4(12.5,2.5,0));
    ps.push_back(Vector4(10,-0,0));
    rps1.push_back(REdgeShell(ps));
    rps1.push_back(REdgeShell(Vector4(15,0,-5),Vector4(10,0,-5)));
    rps1.push_back(REdgeShell(Vector4(15,-8,-5),Vector4(10,-8,-5)));
    rps1.push_back(REdgeShell(Vector4(15,-8,-15),Vector4(10,-8,-15)));
    rps1.push_back(REdgeShell(Vector4(15,0,-15),Vector4(10,0,-15)));
    rps1.push_back(REdgeShell(Vector4(15,5,-12),Vector4(10,3,-12)));
    rps1.push_back(REdgeShell(Vector4(12.5,7,-9),Vector4(12.5,2,-9)));
    rps1.push_back(REdgeShell(Vector4(10,7,-5),Vector4(15,5,-5)));
    rps1.push_back(REdgeShell(Vector4(10,5,10),Vector4(15,5,10)));
    //*/

    
    rps2.push_back(REdgeShell(Vector4(5,0,-25),Vector4(0,0,-25)));
    rps2.push_back(REdgeShell(Vector4(5,0,-20),Vector4(0,2,-20)));
    rps2.push_back(REdgeShell(Vector4(3,1,-15),Vector4(3,4,-15)));
    rps2.push_back(REdgeShell(Vector4(0,0,-10),Vector4(5,2,-10)));
    rps2.push_back(REdgeShell(Vector4(0,0,-5),Vector4(5,0,-5)));

    //
    rps2.push_back(REdgeShell(Vector4(0,0,0),Vector4(5,0,0)));
    rps2.push_back(REdgeShell(Vector4(0,0,8),Vector4(5,0,8)));
    rps2.push_back(REdgeShell(Vector4(0,1,10),Vector4(5,1,10)));
    rps2.push_back(REdgeShell(Vector4(1,5,12),Vector4(6,5,12)));    
    rps2.push_back(REdgeShell(Vector4(3,8,9),Vector4(8,8,9)));
    rps2.push_back(REdgeShell(Vector4(6,4,7),Vector4(11,4,7)));
    rps2.push_back(REdgeShell(Vector4(7,0,12),Vector4(12,0,12)));
    rps2.push_back(REdgeShell(Vector4(8,0,15),Vector4(13,0,15)));
    
    
    rps1.finalize();
    rps2.finalize();

    vector<RPieceShell *> pieces;
    pieces.push_back(&rps1);
    pieces.push_back(&rps2);
    pieces.push_back(&rps1);
    track = new RTrack(pieces,25,15);
    
    /*
    piece1 = new RPiece(rps1,25,20,NULL);
    piece2 = new RPiece(rps2,25,20,piece1);
    piece3 = new RPiece(rps1,25,20,piece2);
    
    piece1->Build();
    piece2->Build();
    piece3->Build();
    */
    //piece2 = new RPiece(rps2,25,10,NULL);
    //piece1 = new RPiece(rps1,25,10,piece2);
   
    
    //cerr<<endl;
    //    piece1->DumpGrid();
    //cerr<<endl;
    
    //    makeCheckImage();
//    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    //   glGenTextures(1,&texName);
    // glBindTexture(GL_TEXTURE_2D,texName);

//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
//    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,WIDTH,HEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,checkImage);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
   
    //    glClearAccum(0,0,0,0);
    //glClear(GL_ACCUM_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, 4.0/3, 0.1, 100);
    

    glMatrixMode(GL_MODELVIEW);
 
}

GLfloat p[3] = {0,0,5};
GLfloat c[3] = {0,0,0};

GLfloat a = .5;

int dist = 0;
int wLoc = 2;

int ballDist = 0;

void display() {
       
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    if(viewFlag)
        track->Viewpoint(dist,wLoc);
    else
        myLook();
 
    
    GLfloat light_position[] = {20.0,10.0,0.0,1.0};
    
    glPushMatrix();
    glRotatef(day,0,1,0);
    glTranslatef(light_position[0],light_position[1],light_position[2]); 
    GLfloat origin[] = {0,0,0,1};
  
    glLightfv(GL_LIGHT0, GL_POSITION, origin);
    //glutSolidSphere(3,10,10);
    glPopMatrix();
    /*
    piece1->Render();
    piece2->Render();
    piece3->Render();
    */
    track->RenderAll();
    //    piece2->DisplayBall(ballDist,12);
    
    

    glutSwapBuffers();
    	
}

void reshape(int w, int h) {
    glViewport(0,0,(GLsizei) w, (GLsizei) h);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(60.0,(GLfloat)w/(GLfloat)h,0.1,100.0);
    //glMatrixMode(GL_MODELVIEW);
}



void keyboard(unsigned char key, int x, int y) {
    switch(key) {
    case 'q':
        day = (day+10)%360;
        break;
    case 'y':
        year = (year+5)%360;
        break;
    case 'd':
        wLoc++;
        if(wLoc>49) wLoc = 49;
        viewAngle = (viewAngle+5)%360;
        break;
    case 'a':
        wLoc--;
        if(wLoc<0) wLoc = 0;
        viewAngle = (viewAngle-5)%360;
        break;
    case 'w':
        if(!viewFlag) {
            speed+=.01;
            if(speed>1) speed = 1;
        } else dist++;
        break;
    case 's':
        if(!viewFlag) {
            speed-=.01;
            if(speed<-1) speed = -1;
        } else dist--;
        break;
    case '=':
        viewPos[1]-=.6;
        break;
    case '-':
        viewPos[1]+=.6;
        break;
    case 'v':
        speed = 0;
        viewFlag=!viewFlag;
        break;
    case ' ':
        speed = 0;
        break;
    }
}

int dir = 1;
int ballCounter = 0;

void idle() {
    ballCounter++;
    if(ballCounter%30000==0) {
        ballCounter = 0;
        ballDist+=dir;
        if(ballDist>200||ballDist<-20) dir*=-1;
    }
    
    
    counter+=1;
	if(counter%15000==0) {
        
        //        dist+=dir;
        //if(dist>200||dist<-20) dir*=-1;
        counter = 0;
        day = (day+1)%360;
    	year = (year+1)%360;
        viewPos[0]+=-speed*sin(viewAngle*PI/180);;
        viewPos[2]+=speed*cos(viewAngle*PI/180);
	glutPostRedisplay();
	}
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH|GLUT_ACCUM);
    glutInitWindowSize(500,500);
    glutInitWindowPosition(100,100);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    
    return 0;
}
