
#include "CSCIx229.h"
#include <time.h>
int axes=1;       //  Display axes
int th=0;         //  Azimuth of view angle
int ph=15;         //  Elevation of view angle
int light=1;      //  Lighting
int rep=1;        //  Repitition
double asp=1;     //  Aspect ratio
double dim=8.5;   //  Size of world
int pause = 0;
// Light values
int ambient   =  15;  // Ambient intensity (%)
int specular = 30;
int diffuse   = 30;  // Diffuse intensity (%)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light

// Specular values
float high[] = {.6,.6,.6,1};
float low[] = {.1,.1,.1,1};
float none[] = {0,0,0,1};

// Snow drift locations
float snow[20][20];
float roofsnow = 0.0;
float snowpts[21][21];
int snowlevel = 0;

//Snowflakes
time_t checkpoint = 0;
int snowrate[] = {4,2,1,0,-1,-2,-4};
int snowval = 3; //middle value

#define PI 3.141592653
#define MAX_PARTICLES 10000

float randomFloat(float a, float b){ //taken from stackoverflow
	float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

typedef struct 
{
	int active;
	int redraw;
	float xpos;
	float ypos;
	float zpos;
	float xspeed;
	float yspeed;
	float zspeed;
	float gravity;

}particles;

particles particlearray[MAX_PARTICLES];

static void initParticles(){
	for (int i = 0; i<MAX_PARTICLES;i++){
		particlearray[i].active = 1;
		particlearray[i].redraw = 0;
		particlearray[i].xpos = randomFloat(-5,5);
		particlearray[i].ypos = randomFloat(0,9);
		particlearray[i].zpos = randomFloat(-5,5);
		particlearray[i].gravity = -0.02; //for now?

		particlearray[i].xspeed = randomFloat(-.05,.05);
		particlearray[i].zspeed = randomFloat(-.05,.05);
	}
}

static void initParticle(int i){
	particlearray[i].xpos = randomFloat(-5,5);
	particlearray[i].zpos = randomFloat(-5,5);
	particlearray[i].ypos = randomFloat(4,9);
	particlearray[i].xspeed = randomFloat(-.05,.05);
	particlearray[i].zspeed = randomFloat(-.05,.05);
}

static void drawParticles(int amount){
	for(int i =0; i<amount; i++){
		if(particlearray[i].active ==1){
			particlearray[i].xpos = particlearray[i].xpos + particlearray[i].xspeed;
			particlearray[i].zpos = particlearray[i].zpos + particlearray[i].zspeed;
			particlearray[i].ypos = particlearray[i].ypos + particlearray[i].gravity;
			particlearray[i].xspeed = particlearray[i].xspeed + randomFloat(-.01,.01);
			particlearray[i].zspeed = particlearray[i].zspeed + randomFloat(-.01,.01);
			if(fabsf(particlearray[i].xspeed) > .15){
				particlearray[i].xspeed = particlearray[i].xspeed*.5;
			}
			if(fabsf(particlearray[i].zspeed) > .15){
				particlearray[i].zspeed = particlearray[i].zspeed*.5;
			}

			if(fabsf(particlearray[i].xpos) > 6 || fabsf(particlearray[i].zpos) > 6 || particlearray[i].ypos < 0){
				initParticle(i);
			}


			glBegin(GL_POINTS);
			glColor3f(1,1,1);
			glNormal3d(ylight,10*Cos(zh),10*Sin(zh));
			glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,high);
			glVertex3f(particlearray[i].xpos,particlearray[i].ypos,particlearray[i].zpos);
			glEnd();
		}
	}
}

static void increaseSnow(float amount){
	float x = 0.0;
	if (amount > 0){
		snowlevel++;
	}
	if (amount < 0){
		snowlevel--;
		if(snowlevel < 0){
			snowlevel = 0;
			return;
		}
	}
	roofsnow = (randomFloat(0,amount)*.25) + roofsnow;

	for (int i = 0; i<20; i++){
		for (int j = 0; j<20; j++){
			 x = (float)rand() / (float)RAND_MAX;
			 x = x * amount;

			 snow[i][j] = x + snow[i][j];

		}
	}
	for(int i = 0; i<21; i++){
		for(int j = 0; j<21; j++){
			//corners first
			if(i==0&&j==0){
				snowpts[i][j] = snow[i][j];
				continue;
			}
			if(i==20&&j==0){
				snowpts[i][j] = snow[i-1][j];
				continue;
			}
			if(i==0&&j==20){
				snowpts[i][j] = snow[i][j-1];
				continue;
			}
			if(i==20&&j==20){
				snowpts[i][j] = snow[i-1][j-1];
				continue;
			}
			//sides now
			if(i==0){
				snowpts[i][j] = (snow[i][j] + snow[i][j-1])/2;
				continue;
			}
			if(j==0){
				snowpts[i][j] = (snow[i][j] + snow[i-1][j])/2;
				continue;
			}
			if(i==20){
				snowpts[i][j] = (snow[i-1][j-1] + snow[i-1][j])/2;
				continue;
			}
			if(j==20){
				snowpts[i][j] = (snow[i][j-1] + snow[i-1][j-1])/2;
				continue;
			}
			else{
				snowpts[i][j] = (snow[i][j]+snow[i-1][j]+snow[i][j-1]+snow[i-1][j-1])/4;
			}
		}
	}
}

static void drawsnow(){
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,high);
	glBegin(GL_QUADS);
	glColor3f(.8,.8,.8);
	for (int i = 0; i<20; i++){
		for(int j = 0; j<20; j++){
			glNormal3d(0,1,0);
			glVertex3f((i/2)-5,snowpts[i][j],(j/2)-5);
			glVertex3f(((i+1)/2)-5,snowpts[i+1][j],(j/2)-5);
			glVertex3f(((i+1)/2)-5,snowpts[i+1][j+1],((j+1)/2)-5);
			glVertex3f((i/2)-5,snowpts[i][j+1],((j+1)/2)-5);

		}
	}
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,none);
	for (int i=0;i<20;i++){
		glNormal3d(0,0,-1);
		glVertex3f((i/2)-5,snowpts[i][0],-5);
		glVertex3f(((i+1)/2)-5,snowpts[i+1][0],-5);
		glVertex3f(((i+1)/2)-5,0,-5);
		glVertex3f((i/2)-5,0,-5);

		glNormal3d(0,0,1);
		glVertex3f((i/2)-5,snowpts[i][20],5);
		glVertex3f(((i+1)/2)-5,snowpts[i+1][20],5);
		glVertex3f(((i+1)/2)-5,0,5);
		glVertex3f((i/2)-5,0,5);

		glNormal3d(-1,0,0);
		glVertex3f(-5,snowpts[0][i],(i/2)-5);
		glVertex3f(-5,snowpts[0][i+1],((i+1)/2)-5);
		glVertex3f(-5,0,((i+1)/2)-5);
		glVertex3f(-5,0,(i/2)-5);

		glNormal3d(1,0,0);
		glVertex3f(5,snowpts[20][i],(i/2)-5);
		glVertex3f(5,snowpts[20][i+1],((i+1)/2)-5);
		glVertex3f(5,0,((i+1)/2)-5);
		glVertex3f(5,0,(i/2)-5);
	}
	glEnd();
}

static void cone(float resolution, float height, float radius){
   float coneang = atan(radius/height);

   //triangles??
      glTexCoord2f(0.0,0.0);
   for(float i=0; i<2*PI; i+=resolution){
      glBegin(GL_TRIANGLES);

      glTexCoord2f(cos(i),0.0);

      glNormal3d(radius*cos(i)*cos(coneang), sin(coneang), radius*sin(i)*cos(coneang));
      glVertex3f(radius*cos(i), 0, radius*sin(i));

      glTexCoord2f(cos(i)+resolution,0.0);

      glNormal3d(radius*cos(i+resolution)*cos(coneang), sin(coneang), radius*sin(i+resolution)*cos(coneang));
      glVertex3f(radius*cos(i+resolution), 0, radius*sin(i+resolution));
      glNormal3d(0,1,0);

      glTexCoord2f(cos(i)+(0.5*resolution),1.0);

      glVertex3f(0,height,0);
      glEnd();
   }
   //bottom circle
   glBegin(GL_TRIANGLE_FAN);
   glNormal3d(0, -1, 0);
   glVertex3f(0,0,0);
   for (float i = 2*PI; i >=0; i-=resolution){
      glNormal3d(radius*cos(i), -1, radius*sin(i));      
      glVertex3f(radius*cos(i), 0, radius*sin(i));
   }
   glVertex3f(radius, 0, 0);
   glEnd();
}

static void cylinder(float resolution, float height, float radius){
   glBegin(GL_TRIANGLE_FAN);
   glNormal3d(0, 1, 0);
   glVertex3f(0, height, 0);
   for (float i=0;i <=(2*PI); i+=resolution){
      glNormal3d(radius*cos(i), 1, radius*sin(i));      
      glVertex3f(radius*cos(i), height, radius*sin(i));
   }
   glVertex3f(radius,height,0);
   glEnd();

   glBegin(GL_TRIANGLE_FAN);
   glNormal3d(0, -1, 0);
   glVertex3f(0,0,0);
   for (float i=0;i <=(2*PI); i+=resolution){
      glNormal3d(radius*cos(i), -1, radius*sin(i));
      glVertex3f(radius*cos(i), 0, radius*sin(i));
   }
   glVertex3f(radius, 0, 0);
   glEnd();

   glBegin(GL_QUAD_STRIP);
   for(float i=0; i<=2*PI; i+=resolution){

      glNormal3d(radius*cos(i),0,radius*sin(i));
      glTexCoord2f(cos(i),0.0);
      glVertex3f(radius*cos(i),0,radius*sin(i));

      glNormal3d(radius*cos(i), height,radius*sin(i));
      glTexCoord2f(cos(i),1.0);
      glVertex3f(radius*cos(i), height, radius*sin(i));
   }
   glVertex3f(radius,0,0);
   glVertex3f(radius,height,0);
   glEnd();
}

static void tree(float height, float tradius, float bradius, float x, float y, float z){
   glPushMatrix();
   glTranslated(x,y,z);

   glColor3f(0.545, 0.271, 0.075);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,low);
   cylinder(.1,height*.15,tradius);

   glPopMatrix();
   glPushMatrix();

   glTranslated(x,y+(height*.15),z);
   glColor3f(0.000, 0.392, 0.000);

   cone(.1,height*.85,bradius);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

static void ground(){
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,low);

	glBegin(GL_QUADS);
	glNormal3d(0,1,0);
	glColor3f(0,.392,0);
	for (int i = -5; i<5; i++){
		for (int j = -5; j<5; j++){
			glVertex3d(i,0,j);
			glVertex3d(i+1,0,j);
			glVertex3d(i+1,0,j+1);
			glVertex3d(i,0,j+1);
		}
	}
	glEnd();

}

static void pentagon(float height, float width){
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,low);


   glBegin(GL_POLYGON);

   glNormal3d(0,0,1);

   glTexCoord2f(0.0,0.0);
   glVertex3f(0.5*width,0,0);

   glTexCoord2f(0,0.8);
   glVertex3f(0.5*width,.8*height,0);

   glTexCoord2f(0.5,1.0);
   glVertex3f(0,height,0);

   glTexCoord2f(1.0,.8);
   glVertex3f(-0.5*width,.8*height,0);

   glTexCoord2f(1.0,0.0);
   glVertex3f(-0.5*width,0,0);

   glEnd();
   glDisable(GL_TEXTURE_2D);


}

static void house(){
	glPushMatrix();


	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,low);
	glScaled(1.2,1.2,1.2);

	glBegin(GL_QUADS);
	glNormal3d(0,0,1);
	glColor3f(0.741, 0.718, 0.420);
	glVertex3f(2,0,1.5);
	glVertex3f(2,2,1.5);
	glVertex3f(-2,2,1.5);
	glVertex3f(-2,0,1.5);

	glNormal3d(0,0,-1);
	glVertex3f(2,0,-1.5);
	glVertex3f(2,2,-1.5);
	glVertex3f(-2,2,-1.5);
	glVertex3f(-2,0,-1.5);

	//roof
	glColor3f(0.545,0.271,0.075);
	glNormal3d(0,.75,.1);
	glVertex3f(2,2,1.5);
	glVertex3f(-2,2,1.5);
	glVertex3f(-2,2.5,0);
	glVertex3f(2,2.5,0);

	glNormal3d(0,.75,-.1);
	glVertex3f(2,2,-1.5);
	glVertex3f(-2,2,-1.5);
	glVertex3f(-2,2.5,0);
	glVertex3f(2,2.5,0);

	glNormal3d(0,0,1);
	glVertex3f(.4,0,1.501);
	glVertex3f(-.4,0,1.501);
	glVertex3f(-.4,1.5,1.501);
	glVertex3f(.4,1.5,1.501);

	if (snowlevel>0){
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,none);
		glColor3f(.8,.8,.8);

		glNormal3d(-1,0,0);
		glVertex3f(-2,2,1.5);
		glVertex3f(-2,2+roofsnow,1.5);
		glVertex3f(-2,2.5+roofsnow,0);
		glVertex3f(-2,2.5,0);

		glVertex3f(-2,2,-1.5);
		glVertex3f(-2,2+roofsnow,-1.5);
		glVertex3f(-2,2.5+roofsnow,0);
		glVertex3f(-2,2.5,0);

		glNormal3d(1,0,0);
		glVertex3f(2,2,1.5);
		glVertex3f(2,2+roofsnow,1.5);
		glVertex3f(2,2.5+roofsnow,0);
		glVertex3f(2,2.5,0);

		glVertex3f(2,2,-1.5);
		glVertex3f(2,2+roofsnow,-1.5);
		glVertex3f(2,2.5+roofsnow,0);
		glVertex3f(2,2.5,0);

		glNormal3d(0,0,-1);
		glVertex3f(2,2,-1.5);
		glVertex3f(2,2+roofsnow,-1.5);
		glVertex3f(-2,2+roofsnow,-1.5);
		glVertex3f(-2,2,-1.5);

		glNormal3d(0,0,1);
		glVertex3f(2,2,1.5);
		glVertex3f(2,2+roofsnow,1.5);
		glVertex3f(-2,2+roofsnow,1.5);
		glVertex3f(-2,2,1.5);

		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,high);
//points
		//.5 by 1.5

		glNormal3d(0, .75, -.1);
		glVertex3f(2,2+roofsnow,-1.5);
		glVertex3f(-2,2+roofsnow,-1.5);
		glVertex3f(-2,2.5+roofsnow,0);
		glVertex3f(2,2.5+roofsnow,0);

		glNormal3d(0,.75,.1);
		glVertex3f(2,2+roofsnow,1.5);
		glVertex3f(-2,2+roofsnow,1.5);
		glVertex3f(-2,2.5+roofsnow,0);
		glVertex3f(2,2.5+roofsnow,0);


	}

	glEnd();

	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,high);
	glNormal3d(0,0,1);
	glColor3f(0.529, 0.808, 0.980);

	glVertex3f(0.7,0.5,1.501);
	glVertex3f(1.5,0.5,1.501);
	glVertex3f(1.5,1.5,1.501);
	glVertex3f(0.7,1.5,1.501);

	glVertex3f(-0.7,0.5,1.501);
	glVertex3f(-1.5,0.5,1.501);
	glVertex3f(-1.5,1.5,1.501);
	glVertex3f(-0.7,1.5,1.501);

	glEnd();


	glRotated(90,0,1,0);
	glTranslated(0,0,2);
	glColor3f(0.741, 0.718, 0.420);
	pentagon(2.5,3);
	glTranslated(0,0,-4);
	pentagon(2.5,3);
	glPopMatrix();
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius r
 */
static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glutSolidSphere(1.0,16,16);
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   int tdiffuse = diffuse+(snowval*5);
   int tspecular = specular+(snowval*5);
   int tambient = ambient +(snowval*5);
   if(tambient>30)
   	tambient = 30;
   //  Length of axes
   //  Eye position
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   glEnable(GL_POLYGON_OFFSET_FILL);
   //  Set perspective
   glLoadIdentity();
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors

      float Ambient[]   = {0.01*tambient ,0.01*tambient ,0.01*tambient ,1.0};
      float Diffuse[]   = {0.01*tdiffuse ,0.01*tdiffuse ,0.01*tdiffuse ,1.0};
      float Specular[] = {.01*tspecular,.01*tspecular,.01*tspecular,1};
      //  Light direction
      float Position[]  = {ylight,10*Cos(zh),10*Sin(zh),1};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      ball(Position[0],Position[1],Position[2] , 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);

   }
   else
      glDisable(GL_LIGHTING);
   //  Draw scene

   glPushMatrix();

   ground();
   if (snowlevel>0){
   	drawsnow();
   }
   tree(5,.2,1,4,0,4);
   house();
   tree(5,.2,1,-4,0,2);
   tree(4.5,.1,.9,-2,0,-4);

   if (snowrate[snowval] > 0){
   	drawParticles(2500*snowrate[snowval]);
   }

   glPopMatrix();

   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Paused=%d",th,ph,dim, pause);
   glWindowPos2i(5,25);
   Print("Snow Level=%d",snowrate[snowval]);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   if (pause == 0){
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;

   zh = fmod(30*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}
   int isneg = 1;
   int tick = 0;
   float ftick = 0;
   if (snowrate[snowval] != 0){
   	tick = 4/snowrate[snowval];
   	isneg = tick / abs(tick);
   	ftick = tick/2.0;
   }
   if(time(0) - checkpoint > fabsf(ftick) && tick != 0){
   	checkpoint = time(0);
   	increaseSnow(.05*isneg);
   }

}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(45,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   // increase dim
   else if (ch == '9')
      dim += 0.1;
   //  decrease dim
   else if (ch == '8' && dim>1)
      dim -= 0.1;
   //pause the light
   else if (ch =='p' || ch == 'p')
      pause = (pause+1)%2;

   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  snowfall level
   else if (ch==']'){
   	snowval -= 1;
   	if (snowval < 0)
   		snowval = 0;
   }
   else if (ch=='['){
      snowval += 1;
      if (snowval > 6)
      	snowval = 6;
   }
   //  Repitition
   else if (ch=='+')
      rep++;
   else if (ch=='-' && rep>1)
      rep--;
  else if (ch=='n' || ch=='N')
  	increaseSnow(.1);
   //  Reproject
   Project(45,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(45,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow(“P. Goudy Final Project“);
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   initParticles();
   checkpoint = time(0);

   //  Load textures
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
