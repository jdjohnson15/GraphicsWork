//FILE: import.cpp
//NAME: Jesse Johnson
//DATE: 3/20/2014
//CS 3014
//
//
// This program imports an object from and .obj file titled "object.obj".
//
// The object should be a golden monkey wearing glasses made from NURBS cylinders. 
//
// 90+ point attempt. Could not figure out textures. 
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
// <space bar>:		  reset to default view 
// 
// (esc): quit
// 
//
//


#include <iostream>	
#include "Angel.h" 
			
#include <fstream>
#include <string>



using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


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

mat4         model, view, instance;

const int MAX_POINTS = 400000; //arbitrarily large number of points that can be in the scene

point4 points[MAX_POINTS];
vec3   normals[MAX_POINTS];
vec2   tex_coords[MAX_POINTS];
int num_vertices = 0;

//lights
Light light;

//materials
Material material;
Material blue;

//texture
const int  TextureSize  = 256;
GLuint textures[1];
GLubyte image[TextureSize][TextureSize][3];

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





//---------------------------------------------------------------
//NEW---------------------------------------------------------------
//---------------------------------------------------------------
void getObject(string filename, point4 points[], vec3 normals[], vec2 tex_coords[], int& numPoints);
//pre: 	filename is the name of the .obj file to be importerd.
//	points is the array of vertices that will be stored in the buffer.
//	normals is the array of all of the point normals to be stored in the buffer and used by the shader.
//	tex_coords are the texture coordinates stored to be used by the shader
//	numPoints is the number of vertices in the object(s).
//post: the points and normals of the object will be stored where they need to be stored, and the material for the object will be assigned 
//	as described in the the associated .mtl file. 










//______________________________________________________________
////main program 
//

int main( int argc, char **argv )
{
	
	glutInit( &argc, argv );
	
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Import .obj" );
	init();
	glutKeyboardFunc( keyboard );
	glutDisplayFunc( display );
	glutSpecialFunc( specialKeyboard );
	glutReshapeFunc(reshape);	
	
	glutMainLoop();
	
	return 0;
}


//__________________________________________________________________________________



void init() 
{



	// define lights

	light.ambient =  color4( 0.4, 0.4, 0.4, 1.0 );
    	light.diffuse =  color4( 3.0, 3.0, 3.0, 1.0 );		
    	light.specular = color4( 2.0, 2.0, 2.0, 1.0 );
	light.position = vec4  ( 0.0, 1.0, 1.0, 1.0 );

	//blue
	blue.ambient = color4( 1.0, 1.0, 1.0, 1.0 );
	blue.diffuse = color4( 0.16, 0.015, 0.019, 1.0 );
	blue.specular = color4( 0.5, 0.5, 0.5, 1.0 );
	blue.shininess = 100.0;

	
	//import object

	getObject("object.obj", points, normals, tex_coords, num_vertices);

	
	//---------------------------------------------------------------------------- 
 
	for ( int i = 0; i < TextureSize; i++ ) {
		for ( int j = 0; j < TextureSize; j++ ) {
		    GLubyte c = (((i & 0x8) == 0) ^ ((j & 0x8)  == 0)) * 255;
		    image[i][j][0]  = c;
		    image[i][j][1]  = c;
		    image[i][j][2]  = c;
		}
	}

	// Initialize texture objects
   	glGenTextures( 1, textures );

   	glBindTexture( GL_TEXTURE_2D, textures[0] );
    	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
    	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
   	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

   
    	glActiveTexture( GL_TEXTURE0 );
    	glBindTexture( GL_TEXTURE_2D, textures[0] );
	


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


	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(tex_coords),NULL, GL_STATIC_DRAW );


	//points
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	
	//normals
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);

	//texture coordinates
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(tex_coords), tex_coords);

	// Load shaders and use the resulting shader program
	program = InitShader( "v_import.glsl", "f_import.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

	vNormal = glGetAttribLocation( program, "vNormal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( sizeof(points) ) );

	GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
    	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET( sizeof(points) + sizeof(normals) ) );

		 
	// Retrieve transformation uniform variable locations
	Model_loc = glGetUniformLocation( program, "Model" );
	View_loc = glGetUniformLocation( program, "View" );

	// Generate the Projection Matrix and send to vertex shader:
	Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Frustum( -1, 1,-1, 1, 1, 10 ) );

	glEnable( GL_DEPTH_TEST );

    	glShadeModel(GL_FLAT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
	}

//__________________________________________________________________________________

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

//__________________________________________________________________________________
void draw()
{


		glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
		applyMaterial(material);
		glDrawArrays( GL_TRIANGLES, 0, num_vertices );

	

}
//__________________________________________________________________________________
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
//__________________________________________________________________________________
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

//__________________________________________________________________________________
void reshape( int w, int h )
{	
	window_width = w;
	window_height = h;
	glutPostRedisplay();
	SetAutoVP(f_l, f_b, f_r, f_t, window_width, window_height);

}

//__________________________________________________________________________________
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

//__________________________________________________________________________________
mat4 flyView(GLdouble cam_x, GLdouble cam_y, GLdouble cam_z){

	mat4 camera = Translate(cam_x, cam_y, cam_z);
	return camera;
}
//__________________________________________________________________________________
mat4 polarView(GLdouble elevation, GLdouble azimuth){

	mat4 camera = Translate(cam_x, cam_y, cam_z)*RotateX(elevation) * RotateY(azimuth)*Translate(-cam_x, -cam_y, -cam_z);
	return camera;
}

//__________________________________________________________________________________
void applyMaterial(Material m)
{
	color4 ambient, diffuse, specular;
	GLfloat shininess; 

	//get products for light 1
	diffuse  = light.diffuse * m.diffuse;
	ambient  = light.ambient * m.ambient;
	specular = light.specular* m.specular;
	glUniform4fv( glGetUniformLocation(program, "diffuse_product"),1, diffuse );
	glUniform4fv( glGetUniformLocation(program, "ambient_product"),1, ambient );
	glUniform4fv( glGetUniformLocation(program, "specular_product"),1,specular);

	glUniform1f( glGetUniformLocation(program,  "Shininess"), m.shininess );
}
	


//__________________________________________________________________________________
void getObject(string filename, point4 points[], vec3 normals[], vec2 tex_coords[], int& numPoints)
{
	string c;
	int normal, vertex, texture;
	GLfloat x, y, z;
	
	point4 tempPoints[MAX_POINTS/3];  // arbitrarily large number of points, 
	vec3  tempNormals[MAX_POINTS/3];  // but can't be more than 1/3 of the total number of points possible in buffer.
	vec2  tempTextures[MAX_POINTS/3];

	//init index vars for normals and vertices 
	int iNormal = 0;
	int iVertex = 0;
	int iTexture = 0;

	//get .obj file
	ifstream infile;
	infile.open(filename.c_str());

	//get rid of garbage info
	string trash;
	getline( infile, trash );
	getline( infile, trash );
	infile >> trash;

	
	// set material-------------------------------------------------------

	//get material data
	ifstream mtlFile;
	infile >> c;
	mtlFile.open(c.c_str());
	
	//get rid of garbage info	
	getline( mtlFile, trash );
	getline( mtlFile, trash );
	getline( mtlFile, trash );

	color4 ambient, diffuse, specular;
	GLfloat shininess, transparency; 

	while (mtlFile >> c)
	{	
		if (c == "Ka")
		{
			mtlFile >> x; 
			mtlFile >> y; 
			mtlFile >> z; 
			ambient = color4(x, y, z, 1.0); 
			
		}
		if (c == "Kd")
		{
			mtlFile >> x; 
			mtlFile >> y; 
			mtlFile >> z; 
			diffuse = color4(x, y, z, 1.0); 
		}
		if (c == "Ks")
		{
			mtlFile >> x; 
			mtlFile >> y; 
			mtlFile >> z; 
			specular = color4(x, y, z, 1.0); 
		}
		if (c == "Ns")
		{
			mtlFile >> shininess; 
		}
		if (c == "d")
		{
			mtlFile >> transparency; 
		}
	} 
		material.ambient = ambient;
		material.diffuse = diffuse;
		material.specular = specular;
		material.shininess = shininess;
			
		
	
	//-------------------------------------------------------------------------------
	
	//the next two lines of data are not needed now, so they are removed
	getline( infile, trash);

	
	//init number of vertices found
	numPoints = 0;

	
	while (infile >> c)
	{	
		if (c == "v")
		{
			infile >> x; 
			infile >> y; 
			infile >> z; 
			tempPoints[iVertex] = point4(x, y, z, 1.0); 
			//std::cout << "v"<<numPoints<<": "<<points[numPoints]<<std::endl;
			++iVertex;
		}
		if (c == "vn")
		{
			infile >> x; 
			infile >> y; 
			infile >> z; 
			tempNormals[iNormal] = vec3(x, y, z);
			++iNormal;
		}
		if (c == "vt")
		{
			infile >> x; 
			infile >> y;  
			tempTextures[iTexture] = vec2(x, y);
			++iTexture;
		}
		if (c == "f")
		{
				for (int i = 0; i < 2 ; ++i)
				{
					getline(infile, c, '/');
					vertex = atoi(c.c_str());

					getline(infile, c, '/');
					texture = atoi(c.c_str());

					getline(infile, c, ' ');
					normal = atoi(c.c_str());

					points[numPoints] = tempPoints[ vertex - 1 ];
					tex_coords[numPoints] = tempTextures[ texture - 1 ];
					normals[numPoints] = tempNormals[ normal - 1 ];
					++numPoints;
				}
					getline(infile, c, '/');
					vertex = atoi(c.c_str());
	
					getline(infile, c, '/');
					texture = atoi(c.c_str());

					getline(infile, c);
					normal = atoi(c.c_str());

					points[numPoints] = tempPoints[ vertex - 1 ];
					tex_coords[numPoints] = tempTextures[ texture - 1 ];
					normals[numPoints] = tempNormals[ normal - 1 ];
					++numPoints;
	
		}
	}

	

}
	







