//FILE: quadrics.cpp
//NAME: Jesse Johnson
//DATE: 12/7/2013
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
mat4         model, instance;

//total point and normal values

//cube variables
const int num_cube_vertices = 36; //36 = (6 faces)*(2triangles/face)*(3 vertices/triangle)

point4 cube_points[num_cube_vertices];
vec3   cube_normals[36];
point4 cube_vertices[8] = {
	point4( -0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5, -0.5, -0.5, 1.0 )
};



//sphere variables
const int sphere_slices = 360;
const int sphere_stacks = 360;
const int num_sphere_vertices = sphere_slices*sphere_stacks*4;
point4 sphere_points[num_sphere_vertices];
vec3 sphere_normals[num_sphere_vertices];
vec3 sphere_face_normals[num_sphere_vertices/4];
float  sphere_radius = 0.5;

//cylinder variables
const int cylinder_slices = 100;
const int cylinder_stacks = 100;
const int num_cylinder_vertices = cylinder_slices*cylinder_stacks*6;
point4 cylinder_points[num_cylinder_vertices];
vec4 cylinder_normals[num_cylinder_vertices];
vec4 cylinder_face_normals[num_cylinder_vertices/6];
float  base_radius = 0.4;
float  top_radius  = 0.45;
float  height	   = 1.0;

//disk variables
const int disk_slices = 50;
const int disk_rings = 50;
const int num_disk_vertices = disk_rings*disk_slices*6;
point4 disk_points[num_disk_vertices];
vec3   disk_normal;
vec3   disk_normals[num_disk_vertices];
float  inRad = 0.0;
float  outRad = 0.45;
float  startAng = 0.0;
float  sweepAng = 360; 

//materials

Material snow;
Material coal;
Material wood;
Material felt;

//lights

Light point_light_1;
Light point_light_2;
Light directional_light;

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
GLdouble zoomin = 2.0;
GLdouble twist    = 0.0;
GLdouble elevation= 0.0;
GLdouble azimuth  = 0.0;


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

mat4 polarView(GLdouble zoomin, GLdouble twist, GLdouble elevation, GLdouble azimuth);
//pre: 	zoomin is how far away the camera is from the subject, 
//	the rest are angles or rotation around the axes:	
//	twist:		z axis 
//	elevation:	x axis
//	azimuth:	y axis
//post: the camera is positioned according to the provided information	

void quad( int a, int b, int c, int d );
//pre:	color is the color id (as identified by the vertex_colors array)
//	a is 1st vertex of face
//	b is 2nd vertex of face
//	c is 3rd vertex of face
//	d is 4th vertex of face
//	(note: a and c share the diagonal of the quad)
//
//post: points for two triangles generated for a quadrilateral face of a solid and saved to arrays (colors and points), and
//      calculates the point normals.

void cube ();
//pre:  
//post: calls quad for each face of cube

void draw();
//pre: cube(), sphere(), cyliner() and disk are called
//post: object is drawn in scene

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
	glutCreateWindow( "Snowman! (ESC to quit)" );
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

	cube ();

	Sphere(sphere_radius, sphere_slices, sphere_stacks, sphere_points, sphere_face_normals, sphere_normals);

	Cylinder(base_radius, top_radius, height, cylinder_slices, cylinder_stacks, cylinder_points, cylinder_face_normals, cylinder_normals);
	
	Disk(outRad, inRad, disk_slices, disk_rings, startAng, sweepAng, disk_points, disk_normal);

	for (int i = 0; i < num_disk_vertices; ++i)
		{disk_normals[i] = disk_normal;}

//----------------------------------------------------------------------------

//define materials
	
	
	//snow
	snow.ambient = color4( 0.7, 0.7, 0.7, 1.0 );
	snow.diffuse = color4( 1.0, 1.0, 1.0, 1.0 );
	snow.specular = color4( 0.2, 0.2, 0.4, 1.0 );
	snow.shininess = 20.0;

	//coal
	coal.ambient = color4( 0.2, 0.2, 0.2, 1.0 );
	coal.diffuse = color4( 0.0, 0.0, 0.0, 1.0 );
	coal.specular = color4( 0.0, 0.0, 0.0, 1.0 );
	coal.shininess = 5.0;

	//felt
	felt.ambient = color4( 0.1, 0.1, 0.1, 1.0 );
	felt.diffuse = color4( 0.2, 0.0, 0.0, 1.0 );
	felt.specular = color4( 0.2, 0.2, 0.2, 1.0 );
	felt.shininess = 5.0;

	//wood
	wood.ambient = color4( 0.2, 0.2, 0.2, 1.0 );
	wood.diffuse = color4( 0.5, 0.2, 0.0, 1.0 );
	wood.specular = color4( 0.0, 0.0, 0.0, 1.0 );
	wood.shininess = 0.001;
	

//define lights

	//point light 1
	point_light_1.ambient =  color4( 0.01, 0.01, 0.01, 1.0 );
    	point_light_1.diffuse =  color4( 0.8, 0.8, 0.8, 1.0 );		
    	point_light_1.specular = color4( 1.0, 1.0, 1.0, 1.0 );
	point_light_1.position = vec4  ( -2.0, -1.0, 1.0, 1.0 );

	//point light 2
	point_light_2.ambient =  color4( 0.01, 0.01, 0.01, 1.0 );
	point_light_2.diffuse =  color4( 0.5, 0.5, 1.0, 1.0 );
	point_light_2.specular = color4( 0.5, 0.5, 0.5, 1.0 );
	point_light_2.position = vec4  ( 0.0, 2.0, 0.0, 1.0 );
	
	//directional light
	directional_light.ambient =  color4( 0.1, 0.1, 0.1, 1.0 );
	directional_light.diffuse =  color4( 0.5, 0.5, 0.6, 1.0 );
	directional_light.specular = color4( 1.0, 1.0, 1.0, 1.0 );
	directional_light.position = vec4  ( 0.0, 1.0, 1.0, 0.0 );

	



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


	glBufferData( GL_ARRAY_BUFFER, sizeof(cube_points) +  sizeof(sphere_points) + sizeof(cylinder_points) + sizeof(disk_points) 
				     + sizeof(cube_normals) + sizeof(sphere_normals)+ sizeof(cylinder_normals)+ sizeof(disk_normals),
		                     NULL, GL_STATIC_DRAW );


	//points
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(sphere_points), sphere_points);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points) + sizeof(sphere_points), sizeof(cylinder_points), cylinder_points);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points) + sizeof(sphere_points) + sizeof(cylinder_points), 
		sizeof(disk_points), disk_points);

	//normals
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points)+sizeof(sphere_points)+sizeof(cylinder_points)+sizeof(disk_points), 
					  sizeof(cube_normals), cube_normals );

	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points)+ sizeof(sphere_points)+sizeof(cylinder_points)+sizeof(disk_points)+
		                          sizeof(cube_normals),sizeof(sphere_normals), sphere_normals);

	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points)+  sizeof(sphere_points)+ sizeof(cylinder_points)+sizeof(disk_points)+
		                          sizeof(cube_normals)+ sizeof(sphere_normals),sizeof(cylinder_normals), cylinder_normals);

	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points)+ sizeof(sphere_points)+ sizeof(cylinder_points)+sizeof(disk_points)+
		                          sizeof(cube_normals)+sizeof(sphere_normals)+sizeof(cylinder_normals), sizeof(disk_normals),disk_normals);

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
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET( sizeof(cube_points)+sizeof(sphere_points)+sizeof(cylinder_points)+sizeof(disk_points) ) );

		 
	// Retrieve transformation uniform variable locations
	Model_loc = glGetUniformLocation( program, "Model" );
	View_loc = glGetUniformLocation( program, "View" );

	// Generate the Projection Matrix and send to vertex shader:
	Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Frustum( -1.0, 1.0, -1.0, 2.0, 1.0, 5.0 ) );



	glEnable( GL_DEPTH_TEST );

    	glShadeModel(GL_FLAT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor( 0.7, 0.7, 0.7, 1.0 ); 
	}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// Generate the View matrix and send to vertex shader:
	glUniformMatrix4fv( View_loc, 1, GL_TRUE, polarView(zoomin, twist, elevation, azimuth) );
	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	glUniform4fv( glGetUniformLocation(program, "light_pos_1"),
		  1, point_light_1.position );

	glUniform4fv( glGetUniformLocation(program, "light_pos_2"),
		  1, point_light_2.position );

	glUniform4fv( glGetUniformLocation(program, "light_pos_dir"),
		  1, directional_light.position );

	
	draw();
	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void draw()
{
	
	//cubes
	mvstack.push(model);
		
		//model
		instance = RotateX(-18)*RotateY(-3)*RotateZ(-12)*Translate(0.0, 0.0, 0.26)*Scale(0.05, 0.05, 0.05);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(coal);
		glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = RotateX(-3)*RotateY(2)*RotateZ(7)*Translate(0.0, 0.0, 0.26)*Scale(0.05, 0.05, 0.05);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(coal);
		glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = RotateX(17)*RotateY(-4)*RotateZ(18)*Translate(0.0, 0.0, 0.26)*Scale(0.05, 0.05, 0.05);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(coal);
		glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = Translate(0.05, 0.4, 0.18)*Scale(0.05, 0.05, 0.05)*RotateZ(-10);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(coal);
		glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = Translate(-0.05, 0.41, 0.18)*Scale(0.05, 0.05, 0.05)*RotateZ(10);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(coal);
		glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = Translate(0.0, 0.35, 0.2)*Scale(0.03, 0.03, 0.15);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(wood);
		glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
		model = mvstack.pop();



	//spheres
	mvstack.push(model);
		
		//model
		instance = Translate(0.0, -0.4, 0.0)*Scale(0.7, 0.6, 0.7);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(snow);
		glDrawArrays( GL_TRIANGLE_STRIP, num_cube_vertices, num_sphere_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = Translate(0.0, 0.0, 0.0)*Scale(0.55, 0.5, 0.55);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(snow);
		glDrawArrays( GL_TRIANGLE_STRIP, num_cube_vertices, num_sphere_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = Translate(0.0, 0.36, 0.0)*Scale(0.4, 0.4, 0.4);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(snow);
		glDrawArrays( GL_TRIANGLE_STRIP, num_cube_vertices, num_sphere_vertices );
		model = mvstack.pop();

	//cylinders
	mvstack.push(model);
		
		//model
		instance = Translate(0.0, 0.49, 0.0)*Scale(0.3, 0.3, 0.3)*RotateX(-90);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(felt);
		glDrawArrays( GL_TRIANGLE_FAN, num_cube_vertices + num_sphere_vertices, num_cylinder_vertices );
		model = mvstack.pop();

	mvstack.push(model);
		
		//model
		instance = Translate(0.0, 0.49, 0.0)*Scale(0.5, 0.06, 0.5)*RotateX(-90);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(felt);
		glDrawArrays( GL_TRIANGLES, num_cube_vertices + num_sphere_vertices, num_cylinder_vertices );
		model = mvstack.pop();

	
	//disks
	mvstack.push(model);

		//model
		instance = Translate(0.0, 0.79, 0.0)*Scale(0.3, 0.3, 0.3)*RotateX(-90);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(felt);
		glDrawArrays( GL_TRIANGLE_FAN, num_cube_vertices + num_sphere_vertices + num_cylinder_vertices, num_disk_vertices );
		model = mvstack.pop();

	mvstack.push(model);

		//model
		instance = Translate(0.0, 0.55, 0.0)*Scale(0.5, 0.5, 0.5)*RotateX(-90);
		model *= instance ;	
		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		//material
		applyMaterial(felt);
		glDrawArrays( GL_TRIANGLE_FAN, num_cube_vertices + num_sphere_vertices + num_cylinder_vertices, num_disk_vertices );
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

//reset all
	case 040: //space key
		zoomin	= 1.0;
		twist		= 2.0;
		elevation	= 0.0;
		azimuth		= 0.0;
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
void quad( int a, int b, int c, int d )
{
	vec4 u = cube_vertices[b] - cube_vertices[a];
    	vec4 v = cube_vertices[c] - cube_vertices[b];

    	vec3 normal = normalize( cross(u, v) );


	cube_normals[Index] = normal; 
	cube_points[Index] = cube_vertices[a]; 
	Index++;

	cube_normals[Index] = normal; 
	cube_points[Index] = cube_vertices[b]; 
	Index++;

	cube_normals[Index] = normal; 
	cube_points[Index] = cube_vertices[c]; 
	Index++;

	cube_normals[Index] = normal; 
	cube_points[Index] = cube_vertices[a]; 
	Index++;

	cube_normals[Index] = normal; 
	cube_points[Index] = cube_vertices[d]; 
	Index++;

	cube_normals[Index] = normal; 
	cube_points[Index] = cube_vertices[c]; 
	Index++;
}

//----------------------------------------------------------------------------
void cube ()
{
    quad( 1, 0, 3, 2 );		//Front 
    quad( 2, 3, 7, 6 );		//Right	
    quad( 3, 0, 4, 7 );		//Bottom 
    quad( 6, 5, 1, 2 );		//Top  
    quad( 4, 5, 6, 7 );		//Back	
    quad( 5, 4, 0, 1 );		//Left  

}

void applyMaterial(Material m)
{
	//get products for light 1
	diffuse  = point_light_1.diffuse * m.diffuse;
	ambient  = point_light_1.ambient * m.ambient;
	specular = point_light_1.specular* m.specular;
	glUniform4fv( glGetUniformLocation(program, "diffuse_product_1"),1, diffuse );
	glUniform4fv( glGetUniformLocation(program, "ambient_product_1"),1, ambient );
	glUniform4fv( glGetUniformLocation(program, "specular_product_1"),1,specular);
	
	//get products for light 2
	diffuse  = point_light_2.diffuse * m.diffuse;
	ambient  = point_light_2.ambient * m.ambient;
	specular = point_light_2.specular* m.specular;
	glUniform4fv( glGetUniformLocation(program, "diffuse_product_2"),1, diffuse );
	glUniform4fv( glGetUniformLocation(program, "ambient_product_2"),1, ambient );
	glUniform4fv( glGetUniformLocation(program, "specular_product_2"),1,specular);

	//get products for directional light
	
	
	diffuse  = directional_light.diffuse * m.diffuse;
	ambient  = directional_light.ambient * m.ambient;
	specular = directional_light.specular* m.specular;
	glUniform4fv( glGetUniformLocation(program, "diffuse_product_dir"),1, diffuse );
	glUniform4fv( glGetUniformLocation(program, "ambient_product_dir"),1, ambient );
	glUniform4fv( glGetUniformLocation(program, "specular_product_dir"),1,specular);


	glUniform1f( glGetUniformLocation(program,  "Shininess"), m.shininess );
}
	







