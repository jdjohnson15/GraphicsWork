//FILE: robotX.cpp
//NAME: Jesse Johnson
//DATE: 11/22/2013
//CS 3014
//
//
// Displays a robot! The camera can be controlled to view the scene.
// It can now move! Animates with keyboard commands. 
//
// 	
// camera/rendering controls:
//
// f: toggles between "lines" and "filled" renderings. 
// 
// left and right arrows: change azimuth of camera
// up and down arrows:    change elevation of camera
// l and r:		  changes the twist of camera
// (+) and (-):		  zoom in to and out of the origin
//
// movement controls:
//
// 	manual:
//		q, Q: left arm pitch (salute)
//		a, A: left arm roll (turn hand)
//		z, Z: left arm yaw (flap)
//		e, E: left elbow
//
//		w, W: right arm pitch
//		s, S: right arm roll
//		x, X: right arm yaw
//		d, D: right elbow
//		
//		y, Y: left leg pitch (kick)
//		h, H: left leg roll (turn foot)
//		n, N: left leg yaw (splits)
//		i, I: left elbow
//	
//		u, U: right leg pitch
//		j, J: right leg roll
//		m, M: right leg yaw
//		k, K: right elbow
//
//		t, T: chest yaw (moves upper body)
//		g, G: core yaw (moves whole body)
//		b, B: hips yaw (moves lower body)
//
//		(the core, chest, and hips have other controls, but I ran out of keyboard buttons that would make sense)
//
//
//	preset animations:
//		Press "1": friendly wave
//		Press "2": chivilrous bow
//		Press "3": dapper tip of the hat
// 
//	
// <space bar>:		  reset to default view and robot position
// 
// (esc): quit
// 
//
//



#include "Angel.h"
#include <assert.h>
#include <iostream>
using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;



////////////
class MatrixStack{
	int index;
	int size;
	mat4* matrices;

	public:
		MatrixStack( int numMat = 32 ): index(0), size(numMat){
			matrices = new mat4[numMat];
		}
		
		~MatrixStack(){
			delete[]matrices;
		}

		void push(const mat4& m) {
			assert(index + 1 < size);
			matrices[index++] = m;
		}

		mat4& pop(){
			assert(index - 1 >= 0);
			index--;
			return matrices[index];
		}
};

//////////

// Model matrix:
MatrixStack  mvstack;
mat4         model;

const int NumVertices = 36; //36 = (6 faces)*(2triangles/face)*(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

//vertices of a unit cube
point4 vertices[8] = {
	point4( -0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[7] = {

	color4( 1.0, 1.0, 1.0, 1.0 ),  // initial null value
	color4( 0.0, 0.0, 0.0, 1.0 ),  // black
	color4( 0.7, 0.7, 0.7, 1.0 ),  // grey
	color4( 0.8, 0.8, 0.8, 1.0 ),  // light grey
	color4( 0.6, 0.6, 0.6, 1.0 ),  // dark grey
	color4( 0.0, 0.5, 1.0, 0.75),  // clear blue
	color4( 1.0, 0.1, 0.1, 1.0 )   // red
};
enum { none = 0, black = 1, grey = 2, light_grey = 3, dark_grey = 4, clear_blue = 5, red = 6 };

// default render-mode toggle boolean:
bool lines = false;

// default animate toggle boolean:
bool animate = false;

// init vertex storage index
int Index = 0;

//init elapsed time values
float t      = 0.0;
float stop_t = 0.0;
float inc    = 0.0;

//storage of previous movement values
float old_rArmX;
float old_rArmY;
float old_rArmZ;
float old_rElbow;
float old_lArmX;
float old_lArmY;
float old_lArmZ;
float old_lElbow;
float old_chestX;
float old_chestZ;
float old_coreY;
float old_hipsY;
float old_hatY;



//animation callsigns
enum { still = 0, wave = 1, bow = 2, hat = 3 };
int preset = still;

// init movment variables

float coreX  = 0;
float coreY  = 0;
float coreZ  = 0;

float chestX = 0;
float chestY = 0;
float chestZ = 0;

float hipsX  = 0;
float hipsY  = 0;
float hipsZ  = 0;

float rArmX  = 0;
float rArmY  = 0;
float rArmZ  =-6;

float lArmX  = 0;
float lArmY  = 0;
float lArmZ  = 6;

float rLegX  = 0;
float rLegY  = 0;
float rLegZ  =-4;

float lLegX  = 0;
float lLegY  = 0;
float lLegZ  = 4;

float headX  = 0;
float headY  = 0;
float headZ  = 0;

float rElbow = 0;
float lElbow = 0;

float rKnee  = 0;
float lKnee  = 0;

float hatX   = 0;
float hatY   = 0;
float hatZ   = 0;







//frame and window default values
float f_l = -1.0; 
float f_b = -1.0; 
float f_r =  1.0;
float f_t =  1.0;
int window_width = 1024;
int window_height = 1024;

//Spherical viewing coordinates
GLdouble zoomin = 1.7;
GLdouble twist    = 0.0;
GLdouble elevation= 0.0;
GLdouble azimuth  = 0.0;


//locations of variable sent to shader
GLuint  Model_loc;  
GLuint  View_loc;  
GLuint  Projection_loc;
GLuint  Color_loc;
GLuint  vPosition;
GLuint  vColor;
GLuint  program;


//______________________________________________________________
//functions//
void init();
void display();
void keyboard(unsigned char key, int x, int y);
void specialKeyboard (int key, int x, int y);
void reshape(int w, int h);
void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight);

mat4 polarView(GLdouble zoomin, GLdouble twist, GLdouble elevation, GLdouble azimuth);
//pre: 	zoomin is how far away the camera is from the subject, 
//	the rest are angles or rotation around the axes:	
//	twist:		z axis 
//	elevation:	x axis
//	azimuth:	y axis
//post: the camera is positioned according to the provided information	

void quad( int color, int a, int b, int c, int d );
//pre:	color is the color id (as identified by the vertex_colors array)
//	a is 1st vertex of face
//	b is 2nd vertex of face
//	c is 3rd vertex of face
//	d is 4th vertex of face
//	(note: a and c share the diagonal of the quad)
//
//post: points for two triangles generated for a quadrilateral face of a solid and saved to arrays (colors and points)

void color_cube (int color_index);
//pre: color_index indicates a color, as identified by the array "vertex_colors" 
//post: vertices of cube object are recolored according to indicated color.

void drawRobot();
//pre: color_cube() is called
//post: the robot, comprised of scaled and translated cubes, is drawn

//________________________________________body parts___________________________

/* the following functions create chains of calls according to how the parts are connect. It starts with the core (ab region) and 
calls functions for connecting parts from there. Typically a function will scale a cube to draw its named body part, create a mat4 variable "instance," which will be a translation accoring to the size of the drawn cube, then call the function for the connected body part, which will use the instance*(current model) value calculated. If a the body part is the end of a chanin (hand or foot), it will not call another function. 

*/



void core();
//pre: drawRobot() is called
//post: chest() and hips() are called

void chest();
//pre: core() is called
//post: robot's chest is drawn, shoulders(), head(), buttons(), and jetpack() are called

void hips();
//pre: core() is called
//post: robot's hip block is drawn, rightLeg() and leftLeg() are called 

void shoulders();
//pre: chest() is called
//post: robot's shoulder axel is drawn, rightArm() and leftArm() are called

void head();
//pre: chest() is called
//post: chest() and hips() are called

void eyes();
//pre: head() is called
//post: chest() and hips() are called

void leg();
//pre: hips() is called
//post: leg is drawn, lowerLeg() is called

void leftArm();
//pre:  shoulders() called
//post: left arm is drawn, lowerLeftArm() is called.

void rightArm();
//pre:  shoulders() called
//post: right arm is drawn, lowerRightArm() is called.

void lowerLeftArm();
//pre: leftArm() is called
//post: lower left arm is drawn, hand() is called

void lowerRightArm();
//pre: rightArm() is called
//post: lower right arm is drawn, hand() is called

void leftHand();
//pre: lowerLeftArm() is called
//post: left hand is drawn, finger() is called 4 times after being rotated and translated accordingly

void rightHand();
//pre: lowerLeftRight() is called
//post: right hand is drawn, finger() is called 4 times after being rotated and translated accordingly

void finger();
//pre: leftHand() or rightHand() is called
//post: finger is drawn

void leftLeg();
//pre: hips() called
//post: left leg is drawn, lowerLeftLeg() is called

void rightLeg();
//pre: hips() called
//post: right leg is drawn, lowerRightLeg() is called

void lowerLeftLeg();
//pre: leftLeg() is called
//post: lower left leg is drawn, foot() is called

void lowerRightLeg();
//pre: rightLeg() is called
//post: lower right leg is drawn, foot() is called

void foot();
//pre: lowerLeg() is called
//post: foot is drawn


//extras_________________________________________

void jet_pack();
//pre: chest() is called
//post: jetpack is drawn

void dapper_hat();
//pre: head is called
//post hat is drawn

void gentlemanly_eyepiece();
//pre: head is called
//post monicle is drawn

void buttons();
//pre: chest() is called
//post: two scaled cubes are drawn, as buttons


//________________________________________animations___________________________

/* 
these functions animate the robot based upon the time from when the function was called
*/

void _wave(float t);
//pre: t is the time variable to tween between two transformations 
//post: robot animated

void _bow(float t);
//pre: t is the time variable to tween between two transformations
//post: robot animated

void _hat(float t);
//pre: t is the time variable to tween between two transformations 
//post: robot animated

//______________________________________________________________
////main program 
//

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Robot! (ESC to quit)" );
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init();
	glutKeyboardFunc( keyboard );
	glutDisplayFunc( display );
	
	glutSpecialFunc( specialKeyboard );
	glutReshapeFunc(reshape);	
	
	glutMainLoop();
	return 0;
}


//______________________________________________________________



void init() 
{
	color_cube ( none );
	// Create a vertex array object
	GLuint vao[1];
	#ifdef __APPLE__       // For use with OS X
	glGenVertexArraysAPPLE(1, vao );
	glBindVertexArrayAPPLE(vao[0] );
	#else		       // Other (Linux)
	glGenVertexArrays(1, vao );
	glBindVertexArray(vao[0] );       
	#endif

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) +  sizeof(colors),
		  NULL, GL_STATIC_DRAW );

	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

	// Load shaders and use the resulting shader program
	program = InitShader( "vshader_robot.glsl", "fshader_robot.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

	vColor = glGetAttribLocation( program, "vColor" ); 
	glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points) ) );

	// set up uniform variables for model, view, and color to be sent to the shader
	Model_loc = glGetUniformLocation( program, "Model" );
	View_loc = glGetUniformLocation( program, "View" );
	Color_loc = glGetUniformLocation( program, "newColor" );
	

	// Generate the Projection Matrix and send to vertex shader:
	Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Frustum( -2.0, 2.0, -2.0, 2.0, 1.0, 5.0 ) );



	glEnable( GL_DEPTH_TEST );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor( 0.5, 0.5, 0.5, 1.0 ); 
	}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// Generate the View matrix and send to vertex shader:
	glUniformMatrix4fv( View_loc, 1, GL_TRUE, polarView(zoomin, twist, elevation, azimuth) );

	//get timing for animations
	t = glutGet(GLUT_ELAPSED_TIME)*0.001 - stop_t;

	if (animate)
	{
		switch (preset){
			case wave:
					_wave(t);
					break;
			case bow:
					_bow(t);
					break;
			case hat:
					_hat(t);
					break;
		}
	}
	if (!animate)
		{stop_t = glutGet(GLUT_ELAPSED_TIME)*0.001;}

	drawRobot();
	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void drawRobot()
{
	//set rotation for whole figure
	mvstack.push(model);
	mat4 instance = RotateX(coreX)*RotateY(coreY)*RotateZ(coreZ);
	model *= instance; 
	core();	
	model = mvstack.pop();
}


void core()
{
	mvstack.push(model);

	//model
	mat4 instance = Scale(0.3, 0.4, 0.3);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	
	model = mvstack.pop();

	//chest rotation
		mvstack.push(model);
		instance = Translate(0.0, 0.23, 0.0)*RotateX(chestX)*RotateY(chestY)*RotateZ(chestZ)*Translate(0.0, -0.23, 0.0);
		instance *= Translate(0.0, 0.25, 0.0);
		model *= instance; 
		chest();	
		model = mvstack.pop();

	//hips rotation
		mvstack.push(model);
		instance = Translate(0.0, -0.18, 0.0)*RotateX(hipsX)*RotateY(hipsY)*RotateZ(hipsZ)*Translate(0.0, 0.18, 0.0);
		instance *= Translate(0.0, -0.2, 0.0);
		model *= instance; 
		hips();	
		model = mvstack.pop();
		
}

void chest()
{
	mvstack.push(model);

	//model
	mat4 instance = Scale(0.4, 0.3, 0.4);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();

	head();
	shoulders();
	jet_pack();
	buttons();

	
}

void buttons()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.15, 0.05, 0.2)*Scale(0.05, 0.05, 0.02);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = red;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();

	mvstack.push(model);

	//model
	instance = Translate(0.08, 0.05, 0.2)*Scale(0.05, 0.05, 0.02);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = red;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();
}

void jet_pack()
{

	//left rocket
	mvstack.push(model);
	//model
	mat4 instance = Translate(0.1, 0.0, -0.25)*Scale(0.15, 0.3, 0.15);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//right rocket
	mvstack.push(model);
	//model
	instance = Translate(-0.1, 0.0, -0.25)*Scale(0.15, 0.3, 0.15);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//middle conector
	mvstack.push(model);
	//model
	instance = Translate(0.0, 0.05, -0.2)*Scale(0.1, 0.1, 0.1);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//left nozzel
	mvstack.push(model);
	//model
	instance = Translate(-0.1, -0.155, -0.25)*Scale(0.1, 0.1, 0.1);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//right nozzel
	mvstack.push(model);
	//model
	instance = Translate(0.1, -0.155, -0.25)*Scale(0.1, 0.1, 0.1);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}
void head()
{
	mvstack.push(model);
	
	mat4 instance = Translate(0.0, 0.275, 0.0)*Scale(0.25, 0.25, 0.25);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, 0.275, 0.0);
	model *= instance ;
	eyes();
		
		//hat wizardry
		mvstack.push(model);
		instance = Translate(hatX, hatY, 0.0)*Translate(0.0, 0.275, 0.0)*RotateZ(hatZ)*Translate(0.0, -0.275, 0.0);
		model *= instance;
		dapper_hat();
		model = mvstack.pop();
	gentlemanly_eyepiece();
	model = mvstack.pop();	
}

void eyes()
{
	mvstack.push(model);

	//modelclear
	mat4 instance = Translate(0.08, -0.02, 0.13)*Scale(0.1, 0.1, 0.05);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = clear_blue;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//other eye
	mvstack.push(model);
	
	//model
        instance = Translate(-0.08, -0.02, 0.13)*Scale(0.1, 0.1, 0.05);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = clear_blue;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();
}

void dapper_hat()
{
	//top
	mvstack.push(model);
	//model
	mat4 instance = Translate(0.0, 0.27, 0.0)*Scale(0.2, 0.27, 0.2);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
	
	//brim
	mvstack.push(model);
	//model
	instance = Translate(0.0, 0.14, 0.0)*Scale(0.27, 0.05, 0.27);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void gentlemanly_eyepiece()
{
	//top frame
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.08, -0.08, 0.15)*Scale(0.12, 0.02, 0.02);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//left frame
	mvstack.push(model);

	//model
	instance = Translate(0.14, -0.02, 0.15)*Scale(0.02, 0.12, 0.05);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//right frame
	mvstack.push(model);

	//model
	instance = Translate(0.02, -0.02, 0.15)*Scale(0.02, 0.12, 0.05);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//bottom frame
	mvstack.push(model);

	//model
	instance = Translate(0.08, 0.04, 0.15)*Scale(0.12, 0.02, 0.02);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

}



void shoulders()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, 0.05, 0.0)*Scale(0.5, 0.1, 0.1);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = black;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.05, 0.0);
	model *= instance ;

		//rotation for right arm 
		mvstack.push(model);

		
		instance  = Translate(-0.28, 0.12, 0.0)*RotateX(rArmX)*RotateZ(rArmZ)*Translate(0.28, -0.12, 0.0);
		instance *= Translate(-0.3, 0.0, 0.0);
		model *= instance;
		rightArm();
		model = mvstack.pop();
	
		//rotation for left arm 
		mvstack.push(model);

		instance  = Translate(0.28, 0.12, 0.0)*RotateX(lArmX)*RotateZ(lArmZ)*Translate(-0.28, -0.12, 0.0);
		instance *= Translate(0.3, 0.0, 0.0);
		model *= instance ;
		leftArm();
		model = mvstack.pop();

	model = mvstack.pop();
}

void leftArm(){

	mvstack.push(model);

	//model
	mat4 instance = Scale(0.13, 0.3, 0.13);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
		//lower arm rotation
		instance = Translate(0.0, -0.12, 0.0)*RotateY(lArmY)*RotateX(lElbow)*Translate(0.0, 0.12, 0.0);
		instance *= Translate(0.0, -0.15, 0.0);
		model *= instance ;
		lowerLeftArm();
		
	model = mvstack.pop();
}

void lowerLeftArm(){

	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.15, 0.0)*Scale(0.2, 0.3, 0.2);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = light_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.15, 0.0);
	model *= instance ;
	leftHand();
	model = mvstack.pop();
}

void leftHand()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.22, 0.0)*Scale(0.05, 0.14, 0.14);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.29, -0.04);
	model *= Translate(0.0, -0.29, -0.04)*RotateX(5)*RotateZ(-8)*Translate(0.0, 0.29, 0.04)*instance ;
	finger();
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.29, 0.0);
	model *= Translate(0.0, -0.29, -0.04)*RotateX(0)*RotateZ(-8)*Translate(0.0, 0.29, 0.04)*instance ;
	finger();
	model = mvstack.pop();
	
	mvstack.push(model);
	instance = Translate(0.0, -0.29, 0.04);
	model *= Translate(0.0, -0.29, -0.04)*RotateX(-5)*RotateZ(-8)*Translate(0.0, 0.29, 0.04)*instance ;
	finger();
	model = mvstack.pop();
	
	mvstack.push(model);
	instance = Translate(-0.04, -0.25, 0.06);
	model *= Translate(0.04, -0.25, -0.06)*RotateX(-10)*RotateZ(-10)*Translate(-0.04, 0.25, 0.06)*instance ;
	finger();
	model = mvstack.pop();
	

}

void rightArm(){

	mvstack.push(model);

	//model
	mat4 instance = Scale(0.13, 0.3, 0.13);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
		//lower arm rotation
		instance = Translate(0.0, -0.12, 0.0)*RotateY(rArmY)*RotateX(rElbow)*Translate(0.0, 0.12, 0.0);
		instance *= Translate(0.0, -0.15, 0.0);
		model *= instance ;
		lowerRightArm();
	model = mvstack.pop();
}

void lowerRightArm(){

	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.15, 0.0)*Scale(0.2, 0.3, 0.2);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = light_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.15, 0.0);
	model *= instance ;
	rightHand();
	model = mvstack.pop();
}

void rightHand()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.22, 0.0)*Scale(0.05, 0.14, 0.14);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.29, -0.04);
	model *= Translate(0.0, -0.29, -0.04)*RotateX(5)*RotateZ(8)*Translate(0.0, 0.29, 0.04)*instance ;
	finger();
	model = mvstack.pop();

	mvstack.push(model);
	instance = Translate(0.0, -0.29, 0.0);
	model *= Translate(0.0, -0.29, -0.04)*RotateX(0)*RotateZ(8)*Translate(0.0, 0.29, 0.04)*instance ;
	finger();
	model = mvstack.pop();
	
	mvstack.push(model);
	instance = Translate(0.0, -0.29, 0.04);
	model *= Translate(0.0, -0.29, -0.04)*RotateX(-5)*RotateZ(8)*Translate(0.0, 0.29, 0.04)*instance ;
	finger();
	model = mvstack.pop();
	
	mvstack.push(model);
	instance = Translate(0.04, -0.25, 0.06);
	model *= Translate(-0.04, -0.25, -0.06)*RotateX(-10)*RotateZ(10)*Translate(0.04, 0.25, 0.06)*instance ;
	finger();
	model = mvstack.pop();
	
}

void finger()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.065, 0.0)*Scale(0.04, 0.13, 0.04);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void hips()
{
	mvstack.push(model);

	//model
	mat4 instance = Scale(0.35, 0.2, 0.35);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();
	mvstack.push(model);
	instance = Translate(0.0, -0.1, 0.0);
	model *= instance;

		//rotation for left leg
		mvstack.push(model);

		instance  = Translate(0.08, 0.14, 0.0)*RotateX(lLegX)*RotateY(lLegY)*RotateZ(lLegZ)*Translate(-0.08, -0.14, 0.0);
		instance *= Translate(0.1, 0.0, 0.0);
		model *= instance;
		leftLeg();
		model = mvstack.pop();

		//rotation for right leg
		mvstack.push(model);

		instance  = Translate(-0.08, 0.14, 0.0)*RotateX(rLegX)*RotateY(rLegY)*RotateZ(rLegZ)*Translate(0.08, -0.14, 0.0);
		instance *= Translate(-0.1, 0.0, 0.0);
		model *= instance;
		rightLeg();
		model = mvstack.pop();

	model = mvstack.pop();
}

void leftLeg()
{

	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.14, 0.0)*Scale(0.18, 0.28, 0.18);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	//lower leg rotation (knee bending)
		instance = Translate(0.0, -0.26, 0.0)*RotateX(lKnee)*Translate(0.0, 0.26, 0.0);
		instance *= Translate(0.0, -0.28, 0.0);
		model *= instance ;
		lowerLeftLeg();
	model = mvstack.pop();
}

void lowerLeftLeg(){

	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.19, 0.0)*Scale(0.25, 0.38, 0.25);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = light_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	//model (lower arm rotation)
	instance = Translate(0.0, -0.38, 0.0);
	model *= instance ;
	foot();
	model = mvstack.pop();
}

void rightLeg()
{

	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.14, 0.0)*Scale(0.18, 0.28, 0.18);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	//lower leg rotation (knee bending)
		instance = Translate(0.0, -0.26, 0.0)*RotateX(rKnee)*Translate(0.0, 0.26, 0.0);
		instance *= Translate(0.0, -0.28, 0.0);
		model *= instance ;
		lowerRightLeg();
	model = mvstack.pop();
}

void lowerRightLeg(){

	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.19, 0.0)*Scale(0.25, 0.38, 0.25);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = light_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	mvstack.push(model);
	//model (lower arm rotation)
	instance = Translate(0.0, -0.38, 0.0);
	model *= instance ;
	foot();
	model = mvstack.pop();
}
void foot()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.0, -0.06, 0.05)*Scale(0.18, 0.12, 0.3);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

//----------------------------------------------------------------------------


void _wave (float _t)
{
	if (_t >= 0.5 && _t < 1.5)
	{
		inc = -180/1;
		rArmX = inc*(_t-0.5);

		inc = 90/1;
		rArmY = inc*(_t-0.5);

		inc = -60/1;
		rArmZ = inc*(_t-0.5);

		inc = 30/1;
		lArmZ = inc*(_t-0.5);

		inc = -20/1;
		chestZ =  inc*(_t-0.5);

		glutPostRedisplay();
		old_rArmX = rArmX;
		old_rArmY = rArmY;
		old_rArmZ = rArmZ;
		old_lArmZ = lArmZ;
		old_chestZ = chestZ;
	}
	
	if (_t >= 1.5 && _t < 2.0)
	{
		inc = 5/.5;
		rArmZ = old_rArmZ + inc*(_t-1.5);
		old_rArmZ = rArmZ;
		glutPostRedisplay();
	}
	if (_t >= 2.0 && _t < 2.5)
	{
		inc = -10/.5;
		rArmZ = old_rArmZ + inc*(_t-2.0);
		old_rArmZ = rArmZ;
		glutPostRedisplay();
	}

	if (_t >= 2.5)
	{
		inc = -old_rArmX/1;
		rArmX = old_rArmX + inc*(_t-2.5);

		inc = -old_rArmY/1;
		rArmY = old_rArmY + inc*(_t-2.5);

		inc = -old_rArmZ/1;
		rArmZ = old_rArmZ + inc*(_t-2.5);

		inc = -old_lArmZ/1;
		lArmZ = old_lArmZ + inc*(_t-2.5);


		inc = -old_chestZ/1;
		chestZ = old_chestZ + inc*(_t-2.5);
		glutPostRedisplay();
	}
	if (_t > 3.5)
	{
		animate = false;
		lArmZ = 6;
		rArmX = 0;
		rArmY = 0;
		rArmZ = -6;
		glutPostRedisplay();
		
	}

}

void _bow (float _t)
{
	if (_t >= .5 && _t < 1.0)
	{
		inc = -60/0.5;
		rArmX = inc*(_t-0.5);

		inc = -90/0.5;
		rArmY = inc*(_t-0.5);

		inc = -10/0.5;
		rArmZ = inc*(_t-0.5);

		inc = 90/0.5;
		rElbow = inc*(_t-0.5);

		inc = 20/0.5;
		lArmX = inc*(_t-0.5);

		inc = 90/0.5;
		lArmY = inc*(_t-0.5);

		inc = 10/0.5;
		lArmZ = inc*(_t-0.5);

		inc = 50/0.5;
		lElbow = inc*(_t-0.5);

		inc = 20/0.5;
		chestX =  inc*(_t-0.5);

		glutPostRedisplay();
		
		old_rArmX = rArmX;
		old_rArmY = rArmY;
		old_rArmZ = rArmZ;
		old_rElbow = rElbow;
		old_lArmX = lArmX;
		old_lArmY = lArmY;
		old_lArmZ = lArmZ;
		old_lElbow = lElbow;
		old_chestX = chestX;
		
		


	}	
	if (_t>= 1.0 && _t < 1.5)
	{
		glutPostRedisplay();
	}
	if (_t >= 1.5 && _t < 3.0)
	{

		inc = -40/1.5;
		coreY = inc*(_t-1.5);

		inc = 40/1.5;
		hipsY = inc*(_t-1.5);

		inc = -old_rArmX/1.5;
		rArmX = old_rArmX + inc*(_t-1.5);

		inc = -old_rArmY/1.5;
		rArmY = old_rArmY + inc*(_t-1.5);

		inc = -old_rArmZ/1.5;
		rArmZ = old_rArmZ + inc*(_t-1.5);

		inc = -old_rElbow/1.5;
		rElbow = old_rElbow + inc*(_t-1.5);

		inc = -old_lArmX/1.5;
		lArmX = old_lArmX + inc*(_t-1.5);

		inc = -old_lArmY/1.5;
		lArmY = old_lArmY + inc*(_t-1.5);

		inc = -old_lArmZ/1.5;
		lArmZ = old_lArmZ + inc*(_t-1.5);

		inc = -old_lElbow/1.5;
		lElbow = old_lElbow + inc*(_t-1.5);

		inc = -old_chestX/1.5;
		chestX =  old_chestX + inc*(_t-1.5);
		glutPostRedisplay();
		
		old_coreY = coreY;
		old_hipsY = hipsY;
		
	}

	if (_t >= 3.0 && _t < 3.5)
	{
		inc = 20/0.5;
		rArmX = inc*(_t-3.0);

		inc = -90/0.5;
		rArmY = inc*(_t-3.0);

		inc = -10/0.5;
		rArmZ = inc*(_t-3.0);

		inc = 50/0.5;
		rElbow = inc*(_t-3.0);

		inc = -60/0.5;
		lArmX = inc*(_t-3.0);

		inc = 90/0.5;
		lArmY = inc*(_t-3.0);

		inc = 10/0.5;
		lArmZ = inc*(_t-3.0);

		inc = 90/0.5;
		lElbow = inc*(_t-3.0);

		inc = 20/0.5;
		chestX =  inc*(_t-3.0);

		glutPostRedisplay();

		old_rArmX = rArmX;
		old_rArmY = rArmY;
		old_rArmZ = rArmZ;
		old_rElbow = rElbow;
		old_lArmX = lArmX;
		old_lArmY = lArmY;
		old_lArmZ = lArmZ;
		old_lElbow = lElbow;
		old_chestX = chestX;
	}

	if (_t>= 3.5 && _t < 4.0)
	{
		glutPostRedisplay();
	}

	if (_t >= 4.0 && _t < 5.0)
	{

		inc = -old_coreY/1.0;
		coreY = old_coreY + inc*(_t-4.0);

		inc = -old_hipsY/1.0;
		hipsY = old_hipsY + inc*(_t-4.0);

		inc = -old_rArmX/1.0;
		rArmX = old_rArmX + inc*(_t-4.0);

		inc = -old_rArmY/1.0;
		rArmY = old_rArmY + inc*(_t-4.0);

		inc = -old_rArmZ/1.0;
		rArmZ = old_rArmZ + inc*(_t-4.0);

		inc = -old_rElbow/1.0;
		rElbow = old_rElbow + inc*(_t-4.0);

		inc = -old_lArmX/1.0;
		lArmX = old_lArmX + inc*(_t-4.0);

		inc = -old_lArmY/1.0;
		lArmY = old_lArmY + inc*(_t-4.0);

		inc = -old_lArmZ/1.0;
		lArmZ = old_lArmZ + inc*(_t-4.0);

		inc = -old_lElbow/1.0;
		lElbow = old_lElbow + inc*(_t-4.0);

		inc = -old_chestX/1.0;
		chestX =  old_chestX + inc*(_t-4.0);
		glutPostRedisplay();
		
	}
	if (_t >= 5.0)
	{
		animate = false;
		glutPostRedisplay();
	}

}

void _hat (float _t)
{
	if (_t >= 0.5 && _t < 1.5)
	{
		inc = -170/1;
		rArmX = inc*(_t-0.5);

		inc = -90/1;
		rArmY = inc*(_t-0.5);

		inc = -45/1;
		rArmZ = inc*(_t-0.5);

		inc = 90/1;
		rElbow = inc*(_t-0.5);

		glutPostRedisplay();

		old_rArmZ = rArmZ;
	}

	if (_t>= 1.5 && _t < 2.0)
	{
		glutPostRedisplay();
	}

	if (_t >= 2.0 && _t < 2.5)
	{
		

		inc = -2/.5;
		rArmZ = old_rArmZ + inc*(_t-2.0);

		inc = -.3/.5;
	        hatX = inc*(_t-2.0);

		inc = .03/.5;
		hatY = inc*(_t-2.0);

		inc = 80/.5;
		hatZ = inc*(_t-2.0);
		

		glutPostRedisplay();
		old_rArmX = rArmX;
		old_rArmY = rArmY;
		old_rArmZ = rArmZ;
		old_rElbow = rElbow;
	}
	
	
	if (_t >= 2.5)
	{
		animate = false;
		glutPostRedisplay();
		
	}
}
//----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{

//camera controls
    switch( key ) {
	case 033: // Escape Key
	    exit( EXIT_SUCCESS );
	    break;

	case 'f':
		if (lines){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			lines = false;
		}else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			lines = true;
		}
		glutPostRedisplay();
		break;

	
	case 'l':
		twist 		+=  5.0;
		glutPostRedisplay();
		break;
	case 'r':
		twist 		+= -5.0;
		glutPostRedisplay();
		break;
	case '+':
		zoomin	+= -0.1;
		glutPostRedisplay();
		break;
	case '-':
		zoomin	+=  0.1;
		glutPostRedisplay();
		break;



//manual movement controls

//torso
	//core (whole body)
	case 'g':
		coreZ   +=  10.0;
		if (coreZ > 30){coreZ = 30;}
		glutPostRedisplay();
		break;

	case 'G':
		coreZ     += -10.0;
		if (coreZ < -30){coreZ = -30;}
		glutPostRedisplay();
		break;
	//chest (upper body)

	case 't':
		chestZ    +=  10.0;
		if (chestZ > 30){chestZ = 30;}
		glutPostRedisplay();
		break;

	case 'T':
		chestZ    += -10.0;
		if (chestZ < -30){chestZ = -30;}
		glutPostRedisplay();
		break;

	//hips (lower body)

	case 'b':
		hipsZ    +=  -10.0;
		if (hipsZ < -30){hipsZ = -30;}
		glutPostRedisplay();
		break;

	case 'B':
		hipsZ     += 10.0;
		if (hipsZ > 30){hipsZ = 30;}
		glutPostRedisplay();
		break;

//arms
	//left arm
	case 'q':
		lArmX    +=  10.0;
		glutPostRedisplay();
		break;

	case 'Q':
		lArmX     += -10.0;
		glutPostRedisplay();
		break;

	case 'a':
		lArmY    +=  10.0;
		if (lArmY > 90){lArmY = 90;}
		glutPostRedisplay();
		break;

	case 'A':
		lArmY     += -10.0;
		if (lArmY < -90){lArmY = -90;}
		glutPostRedisplay();
		break;

	case 'z':
		lArmZ    +=  -10.0;
		if (lArmZ < 0){lArmZ = 0;}
		glutPostRedisplay();
		break;

	case 'Z':
		lArmZ     += 10.0;
		if (lArmZ > 120){lArmZ = 120;}
		glutPostRedisplay();
		break;

	case 'E':
		lElbow    +=  10.0;
		if (lElbow > 0){lElbow = 0;}
		glutPostRedisplay();
		break;

	case 'e':
		lElbow     += -10.0;
		if (lElbow < -90){lElbow = -90;}
		glutPostRedisplay();
		break;
	
	//right arm

	case 'w':
		rArmX    +=  10.0;
		glutPostRedisplay();
		break;

	case 'W':
		rArmX     += -10.0;
		glutPostRedisplay();
		break;

	case 's':
		rArmY    +=  10.0;
		if (rArmY > 90){rArmY = 90;}
		glutPostRedisplay();
		break;

	case 'S':
		rArmY     += -10.0;
		if (rArmY < -90){rArmY = -90;}
		glutPostRedisplay();
		break;

	case 'x':
		rArmZ    +=  10.0;
		if (rArmZ > 0){rArmZ = 0;}
		glutPostRedisplay();
		break;

	case 'X':
		rArmZ     += -10.0;
		if (rArmZ < -120){rArmZ = -120;}
		glutPostRedisplay();
		break;

	case 'D':
		rElbow    +=  10.0;
		if (rElbow > 0){rElbow = 0;}
		glutPostRedisplay();
		break;

	case 'd':
		rElbow     += -10.0;
		if (rElbow < -90){rElbow = -90;}
		glutPostRedisplay();
		break;




//legs 

	//left leg
	case 'y':
		lLegX    +=  10.0;
		if (lLegX > 90){lLegX = 90;}
		glutPostRedisplay();
		break;

	case 'Y':
		lLegX     += -10.0;
		if (lLegX < -90){lLegX = -90;}
		glutPostRedisplay();
		break;

	case 'h':
		lLegY    +=  10.0;
		if (lLegY > 90){lLegY = 90;}
		glutPostRedisplay();
		break;

	case 'H':
		lLegY     += -10.0;
		if (lLegY < -90){lLegY = -90;}
		glutPostRedisplay();
		break;

	case 'n':
		lLegZ    += -10.0;
		if (lLegZ < 0){lLegZ = 0;}
		glutPostRedisplay();
		break;

	case 'N':
		lLegZ     += 10.0;
		if (lLegZ > 45){lLegZ = 45;}
		glutPostRedisplay();
		break;


	case 'i':
		lKnee    +=  10.0;
		if (lKnee > 90){lKnee = 90;}
		glutPostRedisplay();
		break;

	case 'I':
		lKnee     += -10.0;
		if (lKnee < 0){lKnee = 0;}
		glutPostRedisplay();
		break;


	//right leg
	case 'u':
		rLegX    +=  10.0;
		if (rLegX > 90){rLegX = 90;}
		glutPostRedisplay();
		break;

	case 'U':
		rLegX     += -10.0;
		if (rLegX < -90){rLegX = -90;}
		glutPostRedisplay();
		break;

	case 'j':
		rLegY    +=  10.0;
		if (rLegY > 90){rLegY = 90;}
		glutPostRedisplay();
		break;

	case 'J':
		rLegY     += -10.0;
		if (rLegY < -90){rLegY = -90;}
		glutPostRedisplay();
		break;

	case 'm':
		rLegZ    +=  10.0;
		if (rLegZ > 0){rLegZ = 0;}
		glutPostRedisplay();
		break;

	case 'M':
		rLegZ     += -10.0;
		if (rLegZ < -45){rLegZ = -45;}
		glutPostRedisplay();
		break;


	case 'k':
		rKnee    +=  10.0;
		if (rKnee > 90){rKnee = 90;}
		glutPostRedisplay();
		break;

	case 'K':
		rKnee     += -10.0;
		if (rKnee < 0){rKnee = 0;}
		glutPostRedisplay();
		break;

//preset animations

	case '1':
		animate = true;
		preset = wave;
		glutPostRedisplay();
		break;

	case '2':
		animate = true;
		preset = bow;
		glutPostRedisplay();
		break;

	case '3':
		animate = true;
		preset = hat;
		glutPostRedisplay();
		break;
		

//reset all (movement and camera
	case 040: //space key
		zoomin	= 1.7;
		twist		= 0.0;
		elevation	= 0.0;
		azimuth		= 0.0;
		
		coreX  = 0;
		coreY  = 0;
		coreZ  = 0;

		chestX = 0;
		chestY = 0;
		chestZ = 0;

		hipsX  = 0;
		hipsY  = 0;
		hipsZ  = 0;

		rArmX  = 0;
		rArmY  = 0;
		rArmZ  =-6;

		lArmX  = 0;
		lArmY  = 0;
		lArmZ  = 6;

		rLegX  = 0;
		rLegY  = 0;
		rLegZ  =-4;

		lLegX  = 0;
		lLegY  = 0;
		lLegZ  = 4;

		headX  = 0;
		headY  = 0;
		headZ  = 0;

		rElbow = 0;
		lElbow = 0;

		rKnee  = 0;
		lKnee  = 0;

		hatX   = 0;
		hatY   = 0;
		hatZ   = 0;
		glutPostRedisplay();
		break;
	
			
    }
}
//----------------------------------------------------------------------------
void specialKeyboard (int key, int x, int y){
	switch( key ) {
	case GLUT_KEY_LEFT:
		azimuth 	+= -5.0;
		glutPostRedisplay();
		break; 	
	case GLUT_KEY_RIGHT:
		azimuth 	+=  5.0;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_UP:
		elevation 	+=  5.0;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_DOWN:
		elevation 	+= -5.0;
		glutPostRedisplay();
		break; 
	}
}

//----------------------------------------------------------------------------
void reshape( int w, int h )
{	
	window_width = w;
	window_height = h;
	glutPostRedisplay();
	SetAutoVP(f_l, f_b, f_r, f_t, window_width, window_height);

}

//----------------------------------------------------------------------------
void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight)
{
	float Frame_a_r  = (r-l)/(t-b);
	float Window_a_r = wWidth/wHeight;	
	float VPx, VPy, VPw, VPh;
	if (Frame_a_r > Window_a_r){
		VPx = 0;
		VPw = wWidth; 
		VPh = VPw / Frame_a_r;
		VPy = (wHeight - VPh)/2;
	}
	if (Frame_a_r <= Window_a_r){
		VPy = 0;
		VPh = wHeight; 
		VPw = VPh * Frame_a_r;
		VPx = (wWidth - VPw)/2;
	}
	glViewport(VPx, VPy, VPw, VPh);

}

mat4 polarView(GLdouble zoomin, GLdouble twist, GLdouble elevation, GLdouble azimuth){

	mat4 camera = Translate(0.0, 0.0, -zoomin) * RotateZ(twist) * RotateX(elevation) * RotateY(-azimuth);
	return camera;
}

//----------------------------------------------------------------------------
void quad( int color, int a, int b, int c, int d )
{
	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[a]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[b]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[c]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[a]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[d]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[c]; 
	Index++;
}

//----------------------------------------------------------------------------
void color_cube (int color_index)
{
    quad( color_index, 1, 0, 3, 2 );		//Front 
    quad( color_index, 2, 3, 7, 6 );		//Right	
    quad( color_index, 3, 0, 4, 7 );		//Bottom 
    quad( color_index, 6, 5, 1, 2 );		//Top  
    quad( color_index, 4, 5, 6, 7 );		//Back	
    quad( color_index, 5, 4, 0, 1 );		//Left 

}












