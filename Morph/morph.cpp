//FILE: morph.cpp
//NAME: Jesse Johnson
//DATE	October, 2013
//
//  Illustrates using the vertex shader to perform basic morphing, and fragment shader to animate colors
//
//  Subject is a bouncing ghost. The time function is a tragectory quation, using the gravitional constant of acceleration 
//  Its eyes move when it goes down. 	
//
//  Pres g to resue animation, press s to pause.
//
//	Refresher on fstream functions provided by http://www.cplusplus.com/doc/tutorial/files/



#include "Angel.h"
#include <fstream>
#include <string>

using namespace Angel;
using namespace std;

////global variables////

//used for shaders
GLuint          time_location;
GLuint		dropTime_location;
GLuint          vertices_A_location;
GLuint          vertices_B_location;
GLuint		vertices_C_location;
GLuint		vertices_A_colors_location;
GLuint		vertices_B_colors_location;
GLuint		vertices_C_colors_location;
GLuint		program;

//frame and window data variables 
int window_width, window_height;
float f_l, f_b, f_r, f_t;

//animation data
bool go = false;		//init flag for stopping/resuming animation
float postStopTime = 0.0;	//init time elapsed after animation last stopped
float instanceTime = 0.0;	//init current time being saved, marking a possible stop time 
float relativeTime = 0.0;	//init time in current loop
float postDropTime = 0.0;	//init time for end of previous loop (when object "hits the ground")
float v    = 4.4145; 		//realtive simulated velocity coefficient
float rate = 0.001; 		//speed of animation (0.001 = real time)

const int numVertices= 23; 	//number of vertices




//FUNCTIONS:
void init();
void display();
void keyboard(unsigned char key, int x, int y);
void idle();
void reshape(int w, int h);

void getWindowAndFrame(char file_name[], float& l, float& b, float& r, float& t, int& wWidth, int& wHeight); 
//pre: 	file_name is the name of a file containing data regarding the real frame and window, references to the left (l), right (r), 
//	bottom (b), and top (t) values of the real frame, and the width (wWidth) and height (wHeight) of the viewing window. 
//
//post:	r, l, t, and b and the wHeight and wWidth have been set to the values given in file_name. 

 
void getPoints(char file_name[], vec4 positionData[], vec4 colorData[]);
//pre: 	file_name is the name of file with vertex data, 
//	positionData[] is an empty array of vertex positions, and  
//	colorData[] is an empty array of vertex colors	
//post: positionData[] and colorData[] have been filled with data gathered from the file_name. 

void normalize(vec4 array[numVertices]);
//pre: 	array[numVertices] is an array of vertex data.
//
//post: array[numVertices] has been normalized to a [-1, 1] range.

void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight);
//pre: the left (l), right (r), bottom (b), and top (t) values of the real frame, 
//     and the width (wWidth) and height (wHeight) of the viewing window.
//
//post: the viewport has been set according to l, r, b, and t. 

void drawDesign();

//pre: none
//
//post: design will be drawn to screen

//
///main program
//
int main(int argc, char** argv)
{
	getWindowAndFrame("config_frame_and_window.txt", f_l, f_b, f_r, f_t, window_width, window_height);	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE );  
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("The Super Amazing Bouncing Ghost! (g to go, s to stop)");
	glutDisplayFunc(display); 
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	init();
	glutMainLoop();
	return 0;
	}

///end of main




void init()
{
	//get frame and window values
	
	
	//make arrays of positions and colors of vertices_A

	vec4 vertices_A[numVertices];
	
	vec4 vertices_A_colors[numVertices];

	getPoints("config_A.txt", vertices_A, vertices_A_colors);

	//make arrays of positions and colors of vertices_A

	vec4 vertices_B[numVertices];
	
	vec4 vertices_B_colors[numVertices];

	getPoints("config_B.txt", vertices_B, vertices_B_colors);

	//make arrays of positions and colors of vertices_A

	vec4 vertices_C[numVertices];
	
	vec4 vertices_C_colors[numVertices];

	getPoints("config_C.txt", vertices_C, vertices_C_colors);

	//normalize the position data

	normalize(vertices_A);
	normalize(vertices_B);
	normalize(vertices_C);


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
	glBufferData( GL_ARRAY_BUFFER, 
		sizeof(vertices_A) + 
		sizeof(vertices_B) + 
		sizeof(vertices_C) + 
		sizeof(vertices_A_colors) + 
		sizeof(vertices_B_colors) + 
		sizeof(vertices_C_colors), 
		NULL,GL_STATIC_DRAW );
	
	//position subdata
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_A), vertices_A);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_A),  sizeof(vertices_B), vertices_B);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_A)+ sizeof(vertices_B), sizeof(vertices_C),  vertices_C);
	
	int positionBuffer = sizeof(vertices_A)+ sizeof(vertices_B) + sizeof(vertices_C);

	//color subdata
	glBufferSubData(GL_ARRAY_BUFFER, positionBuffer, sizeof(vertices_A_colors), vertices_A_colors);
	glBufferSubData(GL_ARRAY_BUFFER, positionBuffer + sizeof(vertices_A_colors),  sizeof(vertices_B_colors), vertices_B_colors);
	glBufferSubData(GL_ARRAY_BUFFER, positionBuffer + sizeof(vertices_A_colors) + sizeof(vertices_B_colors), sizeof(vertices_C_colors), 			vertices_C_colors);

	//load shaders and use the resulting shader program
	GLuint program = InitShader( "vmorph.glsl", "fmorph.glsl" );
	glUseProgram( program );

	//// Initialize the vertex position attribute for the vertex shader////
	GLuint vertices_A_location = glGetAttribLocation( program, "vertices1" );
	glEnableVertexAttribArray( vertices_A_location );
	glVertexAttribPointer( vertices_A_location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	GLuint vertices_B_location = glGetAttribLocation( program, "vertices2" );
	glEnableVertexAttribArray( vertices_B_location );
	glVertexAttribPointer( vertices_B_location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices_A)));

	GLuint vertices_C_location = glGetAttribLocation( program, "vertices3" );
	glEnableVertexAttribArray( vertices_C_location );
	glVertexAttribPointer( vertices_C_location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices_A)+sizeof(vertices_B)));

	
	//// Initialize the vertex color attribute for the vertex shader////
  	GLuint vertices_A_colors_location = glGetAttribLocation( program, "vColors1" );
	glEnableVertexAttribArray( vertices_A_colors_location );
	glVertexAttribPointer( vertices_A_colors_location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(positionBuffer));

	GLuint vertices_B_colors_location = glGetAttribLocation( program, "vColors2" );
	glEnableVertexAttribArray( vertices_B_colors_location );
	glVertexAttribPointer( vertices_B_colors_location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(positionBuffer + sizeof(vertices_A_colors)));

	GLuint vertices_C_colors_location = glGetAttribLocation( program, "vColors3" );
	glEnableVertexAttribArray( vertices_C_colors_location );
	glVertexAttribPointer( vertices_C_colors_location, 4, GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET(positionBuffer + sizeof(vertices_A_colors) + sizeof(vertices_B_colors)));


	//initialze the time uniform for the vertex shader
	time_location = glGetUniformLocation(program, "time");
	dropTime_location = glGetUniformLocation(program, "dropTime");


	glClearColor( 0.0, 0.0, 0.0, 0.0 ); 
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float elapsedTime;
	float s;

	//animation start/stop control
	if (go)
	{
	elapsedTime = glutGet(GLUT_ELAPSED_TIME)-postStopTime;
	instanceTime = elapsedTime;
	}
	if(!go)
	{
	elapsedTime = instanceTime;
	postStopTime = glutGet(GLUT_ELAPSED_TIME)-instanceTime; 
	}

	elapsedTime = elapsedTime*rate;
	
	float dX = (v - 9.81*relativeTime);
	//object going up
	if (dX > -(v))
	{ 	
		s = (v)*relativeTime-9.81*(relativeTime)*(relativeTime)/2.0;
		relativeTime = elapsedTime - postDropTime;
	}
	 
	//reset relative time when "object hits ground"
		if (dX < -(v))
		{
			relativeTime = 0.0;
			postDropTime = elapsedTime;
		}
	
	glUniform1f(time_location, s);
	
	glUniform1f(dropTime_location, 0.1);

	drawDesign();

	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        exit(0);
        break;
    case 's':
	go = false;
	break;
    case 'g':
	go = true;
	break;
    default:
        break;
    }
}

void idle()
{
   glUniform1f(time_location, glutGet(GLUT_ELAPSED_TIME));
   glutPostRedisplay();
}

void reshape( int w, int h )
{
	window_width = w;
	window_height = h;
	glutPostRedisplay();
	SetAutoVP(f_l, f_b, f_r, f_t, window_width, window_height);

}


void getWindowAndFrame(char file_name[], float& l, float& b, float& r, float& t, int& wWidth, int& wHeight)
{

	ifstream inFile (file_name);

	if (inFile.is_open()){
		inFile >> l >> b >> r >> t >> wWidth >> wHeight;
	}
	else{
		cout<<"unable to open file"<<endl; 
		exit(0);
	}
}

void getPoints(char file_name[], vec4 positionData[], vec4 colorData[])
{

	ifstream inFile (file_name);

	int numPoints = 0;
	int j = 0;
	float x, y, z, w;
	float r, g, b, a;
	inFile >> numPoints;
	float array[numPoints];



	if (inFile.is_open()){
		while(j<numPoints){
			inFile >> positionData[j].x >> positionData[j].y >> positionData[j].z >> positionData[j].w;
			++j;
		}
		j = 0;
		
		while(j<numPoints){
			inFile >> colorData[j].x >> colorData[j].y >> colorData[j].z >> colorData[j].w; 
			++j;
		}
		inFile.close();
	}
	else{
		cout<<"unable to open file"<<endl; 
		exit(0);
	}
}

void normalize(vec4 array[])
{
	for (int i = 0; i < numVertices; ++i){
		array[i].x = array[i].x/(f_r-f_l)*2;
		array[i].y = array[i].y/(f_t-f_b)*2;
	}
}
	

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

void drawDesign()
{
glDrawArrays(GL_TRIANGLE_FAN, 0, 15);
glDrawArrays(GL_TRIANGLE_FAN, 15, 4);
glDrawArrays(GL_TRIANGLE_FAN, 19, 4);
}


