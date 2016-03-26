// Project1.cpp
// Travis Payton
// 26 Oct 2009
//
// For CS 301 Fall 2009
// OpenGL Pong using the Mouse with some ASM functions using Visual Studio's in-line asm
// Requires GLUT (using freeGlut.dll)
//
// Using the following functions written by Dr. Glenn G. Chappell:
// computeMouse
// doselect
// mouseHelper
// myReshape
// printBitmap

#include <ctime>
using std::clock; // to seed rand()
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>       // Do this before GL/GLUT includes
using std::exit;
#ifndef __APPLE__
# include <GL/glut.h>    // GLUT stuff, includes OpenGL headers as well
#else
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
#endif


// Global variables
// Window/viewport
const int startwinsize = 400; // Starting window heighth, width = 2*heigth (pixels)
int pWinw, pWinh;             // Current window width, height (pixels)
double wWinleft, wWinright, wWinbottom, wWintop;

// Mouse Window Coord's
double wMousex, wMousey;

// Paddle Window Coord's for calcspin
double oldpaddlex;
double oldpaddley;

// Values for changing velocity of pongball and computer paddle
double difficulty;
double spin;
double oldTime;

// Event Triggers
bool hit;
bool spinning;
bool menu;
bool easy, medium, hard;

// Keyboard
enum { ESCKEY = 27 };         // ASCII value of escape character


// for type ball (pongball)
struct ball{
	double x;
	double y;
	double vx;
    double vy;
	double vxmax;
	double vymax;

};

// for type paddle (left paddle & right paddle)
struct Paddle {
	double y;
	double maxY;
	double minY;
};

ball pongball;
Paddle lpaddle;
Paddle rpaddle;

// __switchSign takes a double by reference and switches it from
// positive to negative and vise versa.  Written in Assembly
extern "C" double switchSign (double switchme);

// This function calculates the magnitude of the spin and
// sets spinning to true
void calcspin()
{
	spin = (oldpaddley - lpaddle.y)*10;
	if (spin != 0.)
		spinning = true;
	hit = false;
}

// Generates a random value  for the y coordinate
double randomy()
{
	 
	return rand()/(double(RAND_MAX)*2);
}

// playpong
// This function is where all the fun stuff happens
// This calls the various __asm functions. (At some later date, playpong() 
// will be entireley written in Assembly) But for time constraints, simple
// in-line assembly wil be used for the basic operations.

void playpong()
{
	lpaddle.y = wMousey;   // <-- TO DO: rewrite simple operations like this into __asm
	const double MAX_ELAPSED = 0.04;
    double currTime = glutGet(GLUT_ELAPSED_TIME)/1000.;
    double elapsedTime = currTime - oldTime;
    if (elapsedTime > MAX_ELAPSED)
        elapsedTime = MAX_ELAPSED;
    oldTime = currTime;
   
	// Also the movement of the ponball will be redone in __asm
	pongball.x += (5*difficulty*pongball.vx)*elapsedTime;
	pongball.y += (5*difficulty*pongball.vy)*elapsedTime;

	//AI
	if (pongball.x > 0)
	{
		if (pongball.y > rpaddle.y)
			rpaddle.y += difficulty*elapsedTime;
		if (pongball.y < rpaddle.y)
			rpaddle.y -= difficulty*elapsedTime;
	}

	// speed check
	if (pongball.vx > pongball.vxmax && pongball.vx > 0)
		pongball.vx = pongball.vxmax;
	if (pongball.vx < -pongball.vxmax && pongball.vx < 0)
		pongball.vx = -pongball.vxmax;
	if (pongball.vy > pongball.vymax && pongball.vy > 0)
		pongball.vy = pongball.vymax;
	if (pongball.vy < -pongball.vymax && pongball.vy < 0)
		pongball.vy = -pongball.vymax;
	
	//apply spin
	if (spinning){
		if (spin > 0)
			pongball.vx += spin*elapsedTime/20;
		else 
			pongball.vx += -spin*elapsedTime/20;
		pongball.vy += spin*elapsedTime/20;
	}

	
	if (wMousey > lpaddle.maxY)
		lpaddle.y = lpaddle.maxY;
	else if (wMousey < lpaddle.minY)
		lpaddle.y = lpaddle.minY;
	
	// Check to see if the pongball is getting close to the top
    if (pongball.y > .7)
	{
		if (pongball.y > wWintop)
		{
			pongball.y = wWintop - (pongball.y - wWintop);
			//pongball.vy = -pongball.vy;  
			switchSign (pongball.vy);
			
			if (spinning)
			{  // this function needs some work.
				if (spin < 0 )
				{
					pongball.vx -= spin*elapsedTime+pongball.vy/2;
					pongball.vy += spin*elapsedTime-pongball.vy/2;
				}
				else
				{
					pongball.vx += -spin*elapsedTime+pongball.vy/2;
					pongball.vy -= spin*elapsedTime+pongball.vy/2;
				}
			}
		}
	}

	//Check for collision at bottom when ball gets close
	if (pongball.y < .8)
	{
		if (pongball.y < wWinbottom)
		{
			pongball.vy = -pongball.vy;
			pongball.y = wWinbottom - (pongball.y - wWinbottom);
			if (spinning){
				if (spin < 0 )
				{
					pongball.vx += -spin*elapsedTime+pongball.vy/2;
					pongball.vy -= spin*elapsedTime-pongball.vy/2;
				}
				else
				{
					pongball.vx -= spin*elapsedTime+pongball.vy/2;
					pongball.vy += spin*elapsedTime+pongball.vy/2;
				}
			}
		}		
	}

	//Check for Collision at right when ball gets close
	if (pongball.x > wWinright - .35)
	{
		if(pongball.x >= wWinright -.255 && pongball.x <= wWinright -.19 && pongball.y <= rpaddle.y+.1 && pongball.y >= rpaddle.y-.1)
		{
			if (spinning)
			{
				spinning = false;
				pongball.vy -= spin*elapsedTime;
			}
			pongball.vx = -pongball.vx;
		}
		if (pongball.x > wWinright)
		{
			// reset ball
			pongball.x = 0.;
			pongball.y = 0.;
			pongball.vx = -.2;
			pongball.vy = randomy();
			spin = 0.;
			spinning = false;
			
		}
	}

	//Check for a collision at left when ball gets close
	if (pongball.x < wWinleft + .3)
	{
		//check for a collision and add spin
		if (pongball.x <= wWinleft +.255 && pongball.x >= wWinleft +.2 && pongball.y <= lpaddle.y+.1 && pongball.y >= lpaddle.y-.1)
		{
			pongball.vx = -pongball.vx;
			oldpaddley = lpaddle.y;
			hit = true;
		}	
		if (hit)
			if (pongball.x > wWinleft + .285 && pongball.x < wWinleft + .3)
				calcspin();

		if (pongball.x < wWinleft)
		{
			// reset
			pongball.x = 0.;
			pongball.y = 0.;
			pongball.vx = .2;
			pongball.vy = randomy();
			spin = 0.;
			hit = false;
			spinning = false;
			
		}
	}
}

// printBitmap
// Prints the given string at the given raster position
//  using GLUT bitmap fonts.
// You probably don't want any rotations in the model/view
//  transformation when calling this function.
void printBitmap(const string msg, double x, double y)
{
    glRasterPos2d(x, y);
    for (string::const_iterator ii = msg.begin();
         ii != msg.end();
         ++ii)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *ii);
    }
}


// draw_with_names
// Draws the scene.
// Loads the proper name (with glLoadName - name stack must not be empty!)
//  before each object.
// Called by display (for display) and doselect (in selection mode).
void draw_with_names()
{
	// Draw Menu
	glLoadIdentity();
	glLoadName(1);
    glColor3d(.0,.0,.0);
	glRectd(-.1, wWintop - .45, .1,wWintop - .5);
	glColor3d(9., 9., 9.);  // Grey Text
	printBitmap("Start", -.1, wWintop - .5);
	printBitmap("Select Difficulty by clicking on it", -.7, wWintop - .8);
    glLoadName(2);
	glColor3d(.0,.0,.0);
	if (easy)
		glColor3d(.5,.5,.5);
	glRectd(-.5, wWintop - .95, -.3, wWintop - 1.);
	glColor3d(1.,1.,1.);
	printBitmap("Easy", -.5,wWintop - 1.);
	glLoadName(3);
	glColor3d(.0,.0,.0);
	if (medium)
		glColor3d(.5,.5,.5);
	glRectd(-.1,wWintop - .95, .2, wWintop - 1.);
	glColor3d(1.,1.,1.);
	printBitmap("Medium", -.1, wWintop - 1.);
	glLoadName (4);
	glColor3d(0.,0.,0.);
	if (hard)
		glColor3d(.5,.5,.5);
	glRectd(.3, wWintop - .95, .5, wWintop - 1.);
	glColor3d(1.,1.,1.);
	printBitmap("Hard",.3,wWintop -1.);
	printBitmap("Esc   Quit", wWinleft, wWintop - .05);
	printBitmap("(Esc while playing brings up menu)",wWinleft, wWintop - .15);
	printBitmap("Use the mouse to move the paddle!", -.6, wWinbottom+.2);
	printBitmap("Try getting the ball to spin!", -.5, wWinbottom+.1);
}


// set_up_coords
// Does our gluOrtho2D to set up a coordinate system.
// Assumes pWinw, pWinh have been set up by reshape.
// Does NOT set the matrix mode or load an identity.
// Called by reshape (for reshape) and doselect (in selection mode).
void set_up_coords()
{
    // We set up coordinate system so that aspect ratios are always correct,
    //  and the region from -1..1 in x & y always just fits in the viewport.
    if (pWinw > pWinh)
    {
        gluOrtho2D(-double(pWinw)/pWinh, double(pWinw)/pWinh, -1., 1.);
    }
    else
    {
        gluOrtho2D( -1., 1., -double(pWinh)/pWinw, double(pWinh)/pWinw);
    }
}


// doselect
// Use selection mode to check hits on a single pixel.
// Given:
//    x, y: pixel position in GLUT mouse-position format.
//    hitList: vector of int's (will be overwritten!).
// Returns:
//    hitList: contains names of hits.
// Calls set_up_coords to do projection.
// Calls draw_with_names to draw & load names.
void doselect(int x, int y, vector<int> & hitList)
{
    const int BUFFERSIZE = 512;
    GLuint selectionBuffer[BUFFERSIZE];

    // Initialize selection mode
    glSelectBuffer(BUFFERSIZE, selectionBuffer);
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(0);                 // Can't have an empty name stack

    // Set up the pick matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();                // Save the projection
    glLoadIdentity();
    // Insert the following three lines just before
    //  your standard projection setup.
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    gluPickMatrix(x, vp[3]-y, 1, 1, vp);
    set_up_coords();               // Setup same projection as in reshape

    // "Draw" the scene (in selection mode, so no actual output;
    //                   I'm not breaking THE RULES!)
    glMatrixMode(GL_MODELVIEW);    // Always go back to model/view mode
    draw_with_names();
    glFlush();                     // Empty the pipeline

    // Done with pick matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();                 // Restore the old projection
    glMatrixMode(GL_MODELVIEW);    // Always go back to model/view mode

    // Done with selection mode
    int hitCount = glRenderMode(GL_RENDER);

    // Put hits into the caller's array, in a simpler format.
    GLuint * p = selectionBuffer;  // Moving ptr into selectionBuffer
    hitList.clear();
    for (int i = 0; i < hitCount; ++i)
    {
        if (p >= selectionBuffer + BUFFERSIZE)
            break;                 // Beyond end of buffer!

        GLuint stackDepth = *p;    // Get name-stack depth for this hit
        p += (3+stackDepth);       // Skip current cell,
                                   //  min & max depth values,
                                   //  and saved stack

        if (stackDepth > 0)
        {
            if (p-1 >= selectionBuffer + BUFFERSIZE)
                break;             // Beyond end of buffer!

            GLuint hitName = *(p-1);
                                   // Name of object that was hit
            hitList.push_back(int(hitName));
                                   // Save name in caller's vector
        }
    }
}


// myDisplay
// The GLUT display function
void myDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);
	if (menu)
	{
    // Draw Menu
    draw_with_names();
	}
	
	if (!menu)
	{
    //draw pongball
	glColor3d(1.,1.,1.);
	glPointSize(5.);
	glLoadIdentity();
	glTranslated(pongball.x,pongball.y,0.);
	glBegin(GL_POINTS);
		glVertex2d(0., 0.);
	glEnd();
	}
	
	//draw paddles
	glLineWidth(4.);
	glLoadIdentity();
	glTranslated(0.,lpaddle.y,0.);
	glBegin(GL_LINES);
		glVertex2d(wWinleft +.2, .1);
		glVertex2d(wWinleft + .2, -.1);
	glEnd();
	glLoadIdentity();
	glTranslated(0.,rpaddle.y,0.);
	glBegin(GL_LINES);
		glVertex2d(wWinright - .2,-.1);
		glVertex2d(wWinright - .2, .1);
	glEnd();
    

    glutSwapBuffers();
}


// myIdle
// The GLUT idle function
void myIdle()
{
	if(!menu)
	{
		glutSetCursor(GLUT_CURSOR_NONE);
	   	playpong();
	}	
	
	glutPostRedisplay();
	// Print OpenGL errors, if there are any (for debugging)
    if (GLenum err = glGetError())
    {
        cerr << "OpenGL ERROR: " << gluErrorString(err) << endl;
    }
}


	
	

// myReshape
// The GLUT reshape function
void myReshape(int w, int h)
{	
    // Set viewport is entire drawing region of window
    glViewport(0, 0, w, h);
    pWinw = w;  // save viewport dimensions in globals
    pWinh = h;

    // Set projection to 2-D orthogonal
    // Coords set so that box -1..1 in both x,y just fits in window
    // World coords of window boundaries save in globals wWin*.
    if (w > h)
    {
        wWinbottom = -1.;
        wWintop = 1.;
        wWinleft = -double(w)/h;
        wWinright = double(w)/h;
    }
    else
    {
        wWinbottom = -double(h)/w;
        wWintop = double(h)/w;
        wWinleft = -1.;
        wWinright = 1.;
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(wWinleft, wWinright, wWinbottom, wWintop);

    glMatrixMode(GL_MODELVIEW);  // Always go back to model/view mode
}



// myKeyboard
// The GLUT keyboard function
void myKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case ESCKEY:  // ESC: Quit
		if(menu){
			exit(0);
			break;
		}
		else{
			glutSetCursor(GLUT_CURSOR_INHERIT);
			menu = true;
			break;
		}

    }
}


// mouseHelper
// Checks for object hits and prints them
void mouseHelper(int x, int y)
{
    vector<int> hitList;
	if(menu)
		doselect(x, y, hitList);

    if (!hitList.empty())
    {
		for (int i=0; i<hitList.size(); ++i){
			switch (hitList[i]){
			  case 1:
				  menu = false;
				  break;
			  case 2:
				  medium = false;
				  hard = false;
				  easy = true;
				  difficulty = .9;
				  break;
			  case 3:
				  easy = false;
				  medium = true;
				  hard = false;
				  difficulty = 1.2;
				  break;
			  case 4:
				  easy = false;
				  medium = false;
				  hard = true;
				  difficulty = 2.5;
				  break;
			}

		}
		
    }
}

void computeMouse(int x, int y)
{
    wMousex = wWinleft + double(x)/pWinw*(wWinright-wWinleft);
    wMousey = wWintop + double(y)/pWinh*(wWinbottom-wWintop);
}

void myPassiveMotion (int x, int y)
{
	computeMouse (x,y);
}

// myMotion
// The GLUT motion function
void myMotion(int x, int y)
{
    
}


// myMouse
// The GLUT mouse function
void myMouse(int button, int state, int x, int y)
{
    if(menu)
		mouseHelper(x, y);
			
}


// init
// Initializes GL states
// Called by main after window creation
void init()
{
    glClearColor(0., 0., 0., 0.);
	glutSetCursor(GLUT_CURSOR_INHERIT);
	srand((unsigned)time(0));
	oldTime = glutGet(GLUT_ELAPSED_TIME)/1000.;
	lpaddle.y = 0;
	lpaddle.maxY = .9; 
    lpaddle.minY = -.9;
	wMousey = 0.;
	pongball.x = 0.;
	pongball.y = 0.;
	pongball.vx = .2;
	pongball.vy = randomy();
	pongball.vxmax = 3;
	pongball.vymax = 3;
	oldpaddley = lpaddle.y;
	spinning = false;
	spin = 0.;
	difficulty = 1;
	menu = true;
	easy = true;
}


int main(int argc, char ** argv)
{
    // Initialize OpenGL/GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Make a window
    glutInitWindowSize(startwinsize*2, startwinsize);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("CS 301 - Project 1 - Pong with the Mouse! using in-line ASM");

    // Initialize GL states & register GLUT callbacks
    init();
    glutDisplayFunc(myDisplay); 
    glutIdleFunc(myIdle);
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(myKeyboard);
    glutMouseFunc(myMouse);
    glutMotionFunc(myMotion);
	glutPassiveMotionFunc(myPassiveMotion);
    // Do something
    glutMainLoop();

    return 0;
}

