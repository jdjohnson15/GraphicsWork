// FILE: lighted_texture.cpp
// NAME: J.Johnson
// Winter, 2014

//	Demonstrates texturemapping the faces of the cube using image textures
//
//	Demonstrates method of viewing based on spherical coordinates.
//	Works best when the object is centered about the origin, or the "look at" point is the origin.
//	
//	Uses Perspective Projection
//
//	Defalt Values for Viewing:
//		- position for the camera is at the origin, looking down the negtive z-axis
//		- "elevation" about the horizon = 0 degrees
//		- "aximuth" = 0 degrees
//		- "twist" = 0 degrees
//		- "distance" = 2.5
//		
// Keys:
//		UP ARROW: 		increases elevation of the camera 1 degree above horizon
//		DOWN ARROW: 	decreases elevation 1 degree
//		LEFT ARROW: 	moves the camera around to the left 1 degree
//		RIGHT ARROW:  	moves the camera around to the right 1 degree
//		Z:  			zooms in 1 unit  
//		z:  			zooms out 1 unit 
//		+:   			increases twist 1 degree CCW
// 		-: 				increases twist 1 degree CW
	

#include "Angel.h" 
	//provides mat.h, which provides Ortho(), LookAt(), and Frustum
#include <fstream>
using namespace std;

//GLOBAL VARIABLES:

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

//model matrix
mat4 model;

// Texture objects and storage for texture image
const int  TextureSize  = 256;
GLuint textures[2];
GLubyte image1[TextureSize][TextureSize][4];
GLubyte image2[TextureSize][TextureSize][4];
GLubyte image3[TextureSize][TextureSize][4];
GLubyte image4[TextureSize][TextureSize][4];
GLubyte image5[TextureSize][TextureSize][4];
GLubyte image6[TextureSize][TextureSize][4];

//Projection matrix:
mat4 p;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];
vec2   tex_coords[NumVertices];
vec3 normals[NumVertices];
int Index = 0;

// Vertices of a unit cube centered at origin, sides aligned with axes
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

//Materials
Material metal;

//Lights
Light light;

// Shader variable locations:
GLuint  Model_loc;  	// Model matrix uniform shader variable location
GLuint  View_loc; 	// View matrix uniform shader variable location
GLuint  Projection_loc; // projection matrix uniform shader variable location
GLuint program;		//program

// Spherical parameters:
GLdouble zoomin = 2.5;
GLdouble elevation = 0.0;
GLdouble azimuth = 0.0;
GLdouble twist = 0.0;

//light t/f
GLint   point_1 = 0;

//----------------------------------------------------------------------------

// FUNCTIONS:

void quad( int a, int b, int c, int d );
// Precondition:  a, b, c, d are indices of the vertices of a face, listed on CCW order
// Postcondition: two triangles for each face have been generated; color of the first vertex
//    has been assigned to be the color of the face.

void colorcube();
// Postcondition:  6 faces for the cube have been generated using 36 vertices, 
//	12 triangles, and 6 colors.
	
mat4 polarView(GLdouble distance, GLdouble twist, GLdouble elevation, GLdouble azimuth);
// Postcondition:  returns ModelView matrix incorporating camera position given
//	by distance, twist, elevation, and azimuth.

void loadImage(string file_name, GLubyte image[TextureSize][TextureSize][4]);
// Precondition:  file_name is a .ppm file in the current directory that has the first 4 lines of the code from .ppm file removed.
//		  image is the moniker the image will have in the program
//		  requires the following global variables to be intitialized: 
//		  const int TextureSize
// Postcondition: the given image file's color data is saved into the array of images used for textures. 

void generateMipmapTexture(int texture_index, GLubyte image[TextureSize][TextureSize][4]); 
// Precondition:  texture_index is the index of the texture stored in the texture array
//		  image is the image to be used for the texture
//		  requires the following global variables to be intitialized: 
//		  const int TextureSize
// Postcondition: the given image file's color data is saved into the array of images used for textures.  

void applyMaterial(Material m);
//pre: m is a material structure with new parameters for ambient, diffuse, specular, and shininess 
//     that will be used to calculate diffuse, ambient, and specular products
//post: new products calculated and sent to the shader

void init();
void display();
void keyboard(unsigned char key, int x, int y);
void mykeys(int key, int x, int y);
void reshape(int width, int height);

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Toa Nuva Cube (with auto mipmapping)" );
    init();
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutSpecialFunc( mykeys );
    glutReshapeFunc( reshape );
    glutMainLoop();
    return 0;
}

//----------------------------------------------------------------------------

void init() 
{
    colorcube();

	// Load the image file to use for textures
	
	loadImage("1.ppm", image1);
	loadImage("2.ppm", image2);
	loadImage("3.ppm", image3);
	loadImage("4.ppm", image4);
	loadImage("5.ppm", image5);
	loadImage("6.ppm", image6);
	
	// Initialize texture objects
        glGenTextures( 6, textures );


	//generate mipmaps
	generateMipmapTexture(0, image1);
	generateMipmapTexture(1, image2);
	generateMipmapTexture(2, image3);
	generateMipmapTexture(3, image4);
	generateMipmapTexture(4, image5);
	generateMipmapTexture(5, image6);

	//define material
	metal.ambient = color4( 0.1, 0.7, 0.7, 1.0 );
	metal.diffuse = color4( 0.1, 1.0, 1.0, 1.0 );
	metal.specular = color4( 1.0, 1.0, 1.0, 1.0 );
	metal.shininess = 10.0;
	
	//define light
	light.ambient =  color4( 0.5, 0.5, 0.5, 1.0 );
    	light.diffuse =  color4( 0.8, 0.8, 0.8, 1.0 );		
    	light.specular = color4( 1.0, 1.0, 1.0, 1.0 );
	light.position = vec4  ( -0.5, 0.5, 0.0, 1.0 );

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
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(tex_coords),
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals), sizeof(tex_coords), tex_coords );

    // Load shaders and use the resulting shader program
    program = InitShader( "vshader_lighted_texture.glsl", "fshader_lighted_texture.glsl" );
    glUseProgram( program );

    // set up vertex arrays
	GLintptr offset = 0; 
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(offset) );
	offset += sizeof(points);

    GLuint vNormals = glGetAttribLocation( program, "vNormals" ); 
    glEnableVertexAttribArray( vNormals );
    glVertexAttribPointer( vNormals, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(offset) );
	offset += sizeof(normals);

	GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(offset) );


    glUniform1i( glGetUniformLocation(program, "texture"), 0 );
    Model_loc = glGetUniformLocation( program, "Model" );
    View_loc = glGetUniformLocation( program, "View" );
    Projection_loc = glGetUniformLocation( program, "Projection" );

    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	

	//Generate the View Matrix and send to vertex shader:	
	glUniformMatrix4fv( View_loc, 1, GL_TRUE, polarView(zoomin, twist, elevation, azimuth));

	//Generate the Model Matrix and send to vertex shader
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//Generate the Projection Matrix and send to vertex shader:
	p = Frustum( -1.0, 1.0, -1.0, 1.0, 1.0, 10.0 );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, p );

	glUniform4fv( glGetUniformLocation(program, "light_pos_1"),1, light.position );


	//apply material
	applyMaterial(metal);


	//draw faces of cube
	glBindTexture( GL_TEXTURE_2D, textures[0] );
	glDrawArrays( GL_TRIANGLES, 0, 6 );

	glBindTexture( GL_TEXTURE_2D, textures[1] );
	glDrawArrays( GL_TRIANGLES, 6, 6 );

	glBindTexture( GL_TEXTURE_2D, textures[2] );
	glDrawArrays( GL_TRIANGLES, 12, 6 );

	glBindTexture( GL_TEXTURE_2D, textures[3] );
	glDrawArrays( GL_TRIANGLES, 18, 6 );

	glBindTexture( GL_TEXTURE_2D, textures[4] );
	glDrawArrays( GL_TRIANGLES, 24, 6 );

	glBindTexture( GL_TEXTURE_2D, textures[5] );
	glDrawArrays( GL_TRIANGLES, 30, 6 );

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void quad( int a, int b, int c, int d )
{

	vec4 u = vertices[b] - vertices[a];
	vec4 v = vertices[c] - vertices[b];

	vec3 normal = normalize( cross(u, v) );

	normals[Index] = normal;
	points[Index] = vertices[a]; tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;

	normals[Index] = normal;
	points[Index] = vertices[b]; tex_coords[Index] = vec2( 0.0, 1.0 );
	Index++;

	normals[Index] = normal;
	points[Index] = vertices[c]; tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;

	normals[Index] = normal;
	points[Index] = vertices[a]; tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;

	normals[Index] = normal;
	points[Index] = vertices[c]; tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;

	normals[Index] = normal;
	points[Index] = vertices[d]; tex_coords[Index] = vec2( 1.0, 0.0 );
	Index++;
}

//----------------------------------------------------------------------------

void colorcube()
{
    quad( 1, 0, 3, 2 );		
    quad( 2, 3, 7, 6 );		
    quad( 3, 0, 4, 7 );		
    quad( 6, 5, 1, 2 );		
    quad( 4, 5, 6, 7 );		
    quad( 5, 4, 0, 1 );		
}

//----------------------------------------------------------------------------

mat4 polarView(GLdouble distance, GLdouble twist, GLdouble elevation, GLdouble azimuth)
{
	mat4 c;
	c = Translate(0.0, 0.0, -distance) * RotateZ(twist) * RotateX(elevation) * RotateY(-azimuth);
	return c; 
}

//----------------------------------------------------------------------------

void loadImage(string file_name, GLubyte image[TextureSize][TextureSize][4])
{
	int i, j, c;

	ifstream infile;
	infile.open(file_name.c_str());
	
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			infile>>c;  image[i][j][0] = (GLubyte) c;
			infile>>c;  image[i][j][1] = (GLubyte) c;
			infile>>c;  image[i][j][2] = (GLubyte) c;
		        image[i][j][3] = (GLubyte) 255;
		}
	}
	
	infile.close();
}

//----------------------------------------------------------------------------

void generateMipmapTexture(int texture_index, GLubyte image[TextureSize][TextureSize][4])
{
	glBindTexture( GL_TEXTURE_2D, textures[texture_index] );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);	//256 is level 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 8); 	//256,128,64,32,16,8,4,2,1

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );

	//Supply the image for base level:

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TextureSize, TextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );

	//Have all the other mipmaps automatically generated: 
	glGenerateMipmap(GL_TEXTURE_2D);
}
//----------------------------------------------------------------------------
void applyMaterial(Material m)
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	diffuse  = light.diffuse * m.diffuse;
	ambient  = light.ambient * m.ambient;
	specular = light.specular* m.specular;
	glUniform4fv( glGetUniformLocation(program, "diffuse_product_1"),1, diffuse );
	glUniform4fv( glGetUniformLocation(program, "ambient_product_1"),1, ambient );
	glUniform4fv( glGetUniformLocation(program, "specular_product_1"),1,specular);

	glUniform1f( glGetUniformLocation(program,  "Shininess"), m.shininess );
}
//----------------------------------------------------------------------------

void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	break;
	case '+': 
		twist = twist + 1.0;
		glutPostRedisplay();
	break;
	case '-':
		twist = twist - 1.0;
		glutPostRedisplay();
	break;
	case 'Z':
		zoomin = zoomin + 0.1;
		glutPostRedisplay();
	break;
	case 'z':
		zoomin = zoomin - 0.1;
		glutPostRedisplay();
	break;
	case ' ':  //space bar - reset everything
		zoomin = 2.5;
		elevation = 0.0;
		azimuth = 0.0;
		twist = 0.0;
		glutPostRedisplay();
	break;

	case '1': 
		if (point_1 == 0){point_1 = 1;}
		else{point_1 = 0;}
		glUniform1i( glGetUniformLocation(program, "point_1"), point_1);
		glutPostRedisplay();
		
	break;
	
    }
}

//----------------------------------------------------------------------------

void mykeys(int key, int x, int y)
{
   switch(key){
           case GLUT_KEY_LEFT:
                   azimuth = azimuth - 5.0;
                   glutPostRedisplay();
           break;
                   
      case GLUT_KEY_RIGHT:
                   azimuth = azimuth + 5.0;
                   glutPostRedisplay();
          break;
                   
      case GLUT_KEY_DOWN:
                   elevation = elevation - 5.0;
                   glutPostRedisplay();
          break;
                   
      case GLUT_KEY_UP:
                   elevation = elevation + 5.0;
                   glutPostRedisplay();
          break;
                  
      default:
          break;
    }
}

//----------------------------------------------------------------------------

