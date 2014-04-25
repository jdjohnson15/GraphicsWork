//FILE: quadrics.cpp
//NAME: Jesse Johnson
//DATE: 1/8/2013
//CS 3014
//
//
// Displays a snowman with the sphere, disk, cube, and cylinder primatives.
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
//	
// <space bar>:		  reset to default view and robot position
// 
// (esc): quit
// 
//
//


#include <iostream>	//allows use of cout and cin (for debug purposes)
#include "Angel.h"
#include "sphere.h" 	//allows use of Sphere()
#include "cylinder.h"	//allows use of Cyliner()
#include "disk.h"	//allows use of Disk()	
#include <assert.h>	//allows use of assert()


using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;



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

struct Material{

		color4 ambient;
		color4 diffuse;
		color4 specular;
		float  shininess;
	
};

struct Light{

		color4 ambient;
		color4 diffuse;
		color4 specular;
		vec4 position;	
};
//------------------------------------------------------------------

MatrixStack mvstack;
mat4         model, view, instance;

//sphere variables
const int sphere_slices = 40;
const int sphere_stacks = 40;
const int num_sphere_vertices = sphere_slices*sphere_stacks*5;
point4 sphere_points[num_sphere_vertices];
vec3 sphere_normals[num_sphere_vertices];
vec3 sphere_face_normals[num_sphere_vertices/4];
float  sphere_radius = 1.0;


//materials

Material red;
Material blue;

//lights

Light light;

//values for light properties
color4 ambient;
color4 diffuse;
color4 specular;

// default render-mode toggle boolean:
bool lines = false;


// init vertex storage index
int Index = 0;

//frame and window default values
float f_l = -1.0; 
float f_b = -1.0; 
float f_r =  1.0;
float f_t =  1.0;
int window_width = 1024;
int window_height = 1024;

//Spherical viewing coordinates
GLdouble elevation= 0.0;
GLdouble azimuth  = 0.0;

//fly viewing coordinates 

GLdouble cam_x = 0.0;
GLdouble cam_y = 0.0;
GLdouble cam_z = -2.0;


//locations of variable sent to shader
GLuint  Model_loc;  
GLuint  View_loc;  
GLuint  Projection_loc;
GLuint  vPosition;
GLuint  vNormal;
GLuint  program;


//______________________________________________________________
//functions//
void init();
void display();
void keyboard(unsigned char key, int x, int y);
void specialKeyboard (int key, int x, int y);
void reshape(int w, int h);
void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight);
void draw();

mat4 flyView(GLdouble cam_x, GLdouble cam_y, GLdouble cam_z);
//pre: 	cam_x, cam_y, and cam_z are the camera's x, y, and z positional coordinates, respectively. 
//post: the camera is positioned according to the provided information	

mat4 polarView(GLdouble elevation, GLdouble azimuth);
//pre: 	
//	elevation:	y axis rotation
//	azimuth:	x axis rotation
//post: the camera is positioned according to the provided information	


void applyMaterial(Material m);
//pre: m is a material structure with new parameters for ambient, diffuse, specular, and shininess 
//     that will be used to calculate diffuse, ambient, and specular products
//post: new products calculated and sent to the shader



//______________________________________________________________
////main program 
//

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "spheres (ESC to quit)" );
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

//----------------------------------------------------------------------------

//call functions to plot points for primatives


	Sphere(sphere_radius, sphere_slices, sphere_stacks, sphere_points, sphere_face_normals, sphere_normals);

//----------------------------------------------------------------------------

//define materials
	
	
	//red
	red.ambient = color4(0.2, 0.2, 0.2, 1.0 );
	red.diffuse = color4( 0.8, 0.0, 0.0, 1.0 );
	red.specular = color4( 1.0, 1.0, 1.0, 1.0 );
	red.shininess = 25.0;

	//blue
	blue.ambient = color4( 0.2, 0.2, 0.2, 1.0 );
	blue.diffuse = color4( 0.0, 0.1, 1.0, 1.0 );
	blue.specular = color4( 0.5, 0.0, 0.0, 1.0 );
	blue.shininess = 100.0;


//define lights

	//light
	light.ambient =  color4( 0.0, 0.0, 0.0, 1.0 );
    	light.diffuse =  color4( 1.0, 1.0, 1.0, 1.0 );		
    	light.specular = color4( 1.0, 1.0, 1.0, 1.0 );
	light.position = vec4  ( 1.0, 1.0, 1.0, 1.0 );

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


	glBufferData( GL_ARRAY_BUFFER, sizeof(sphere_points)+ sizeof(sphere_normals),NULL, GL_STATIC_DRAW );


	//points
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(sphere_points), sphere_points );
	
	//normals
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(sphere_points), sizeof(sphere_normals), sphere_normals);

	// Load shaders and use the resulting shader program
	program = InitShader( "vquadrics.glsl", "fquadrics.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

	vNormal = glGetAttribLocation( program, "vNormal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( sizeof(sphere_points) ) );

		 
	// Retrieve transformation uniform variable locations
	Model_loc = glGetUniformLocation( program, "Model" );
	View_loc = glGetUniformLocation( program, "View" );

	// Generate the Projection Matrix and send to vertex shader:
	Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Frustum( -1, 1,-1, 1, 1, 10 ) );



	glEnable( GL_DEPTH_TEST );

    	glShadeModel(GL_FLAT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
	}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Generate the View matrix and send to vertex shader:
	view = polarView(elevation, azimuth)*flyView(cam_x, cam_y, cam_z);
	glUniformMatrix4fv( View_loc, 1, GL_TRUE, view );

	glUniform4fv( glGetUniformLocation(program, "light_pos"), 1, light.position );

	draw();
	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void draw()
{
	//spheres

	//blue sphere
	mvstack.push(model);
		
		//model
		instance = Translate(0.0, 0.0, 0.0);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(blue);
		glDrawArrays( GL_TRIANGLE_STRIP, 0, num_sphere_vertices );
		model = mvstack.pop();

	//red sphere
	mvstack.push(model);
		
		//model
		instance = Translate(-1.0, -0.5, 1.5);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(red);
		glDrawArrays( GL_TRIANGLE_STRIP, 0, num_sphere_vertices );
		model = mvstack.pop();

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

	
	case 'j':
		azimuth 		+=   5.0;
		glutPostRedisplay();
		break;
	case 'l':
		azimuth			+=  -5.0;
		glutPostRedisplay();
		break;

	case 'i':
		elevation 		+=   5.0;
		glutPostRedisplay();
		break;
	case 'k':
		elevation		+=  -5.0;
		glutPostRedisplay();
		break;

	case 'a':
		cam_y 		        +=   -0.1;
		glutPostRedisplay();
		break;
	case 'z':
		cam_y 		        +=    0.1;
		glutPostRedisplay();
		break;
	



//reset all
	case 040: //space key
		cam_x = 0.0;
		cam_y = 0.0;
		cam_z = -2.0;
		elevation= 0.0;
		azimuth  = 0.0;
		glutPostRedisplay();
		break;
	}
}
//----------------------------------------------------------------------------
void specialKeyboard (int key, int x, int y){
	switch( key ) {
	case GLUT_KEY_LEFT:
		cam_x 	+=   0.1;
		glutPostRedisplay();
		break; 	
	case GLUT_KEY_RIGHT:
		cam_x 	+=  -0.1;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_UP:
		cam_z 	+=   0.1;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_DOWN:
		cam_z 	+=  -0.1;
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

//----------------------------------------------------------------------------
mat4 flyView(GLdouble cam_x, GLdouble cam_y, GLdouble cam_z){

	mat4 camera = Translate(cam_x, cam_y, cam_z);
	return camera;
}
//----------------------------------------------------------------------------
mat4 polarView(GLdouble elevation, GLdouble azimuth){

	mat4 camera = Translate(cam_x, cam_y, cam_z)*RotateX(elevation) * RotateY(azimuth)*Translate(-cam_x, -cam_y, -cam_z);
	return camera;
}

//----------------------------------------------------------------------------


void applyMaterial(Material m)
{
	//get products for light 1
	diffuse  = light.diffuse * m.diffuse;
	ambient  = light.ambient * m.ambient;
	specular = light.specular* m.specular;
	glUniform4fv( glGetUniformLocation(program, "diffuse_product"),1, diffuse );
	glUniform4fv( glGetUniformLocation(program, "ambient_product"),1, ambient );
	glUniform4fv( glGetUniformLocation(program, "specular_product"),1,specular);

	glUniform1f( glGetUniformLocation(program,  "Shininess"), m.shininess );
}
	







