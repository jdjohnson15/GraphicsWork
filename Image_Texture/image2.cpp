// FILE: cube_image.cpp
// NAME: T. Garrett
// Winter, 2014
//	Demonstrates texturemapping the faces of the cube using an image texture
//	Uses image file sunrise_256.ppm, which is found in the /classes/cs3014/files/Textures directory.
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

// Texture objects and storage for texture image
const int  TextureSize  = 256;
GLuint textures[2];
GLubyte image1[TextureSize][TextureSize][3];
GLubyte image2[TextureSize][TextureSize][3];
GLubyte image3[TextureSize][TextureSize][3];
GLubyte image4[TextureSize][TextureSize][3];
GLubyte image5[TextureSize][TextureSize][3];
GLubyte image6[TextureSize][TextureSize][3];


//Projection matrix:
mat4 p;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];
vec2   tex_coords[NumVertices];
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

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

// Shader variable locations:
GLuint  ModelView_loc;  // ModelView matrix uniform shader variable location
GLuint  Projection_loc; // projection matrix uniform shader variable location

// Spherical parameters:
GLdouble zoomin = 2.5;
GLdouble elevation = 0.0;
GLdouble azimuth = 0.0;
GLdouble twist = 0.0;

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
    glutCreateWindow( "Spherical Viewing with Perspective Projection" );
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

	// Load the image file to use for texture
    
	int i, j, c;
	ifstream infile;

	infile.open("1.ppm");
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			 infile>>c;  image1[i][j][0] = (GLubyte) c;
			 infile>>c;  image1[i][j][1] = (GLubyte) c;
			 infile>>c;  image1[i][j][2] = (GLubyte) c;

		}
	}
	infile.close();

	infile.open("2.ppm");
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			 infile>>c;  image2[i][j][0] = (GLubyte) c;
			 infile>>c;  image2[i][j][1] = (GLubyte) c;
			 infile>>c;  image2[i][j][2] = (GLubyte) c;

		}
	}
	infile.close();

	infile.open("3.ppm");
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			 infile>>c;  image3[i][j][0] = (GLubyte) c;
			 infile>>c;  image3[i][j][1] = (GLubyte) c;
			 infile>>c;  image3[i][j][2] = (GLubyte) c;

		}
	}
	infile.close();

	infile.open("4.ppm");
	
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			 infile>>c;  image4[i][j][0] = (GLubyte) c;
			 infile>>c;  image4[i][j][1] = (GLubyte) c;
			 infile>>c;  image4[i][j][2] = (GLubyte) c;

		}
	}
	infile.close();
	
	infile.open("5.ppm");
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			 infile>>c;  image5[i][j][0] = (GLubyte) c;
			 infile>>c;  image5[i][j][1] = (GLubyte) c;
			 infile>>c;  image5[i][j][2] = (GLubyte) c;

		}
	}
	infile.close();

	infile.open("6.ppm");
	 
	for (j = 0; j <TextureSize; j++) 
	{
		for (i = 0; i <TextureSize; i++) 
		{
			 infile>>c;  image6[i][j][0] = (GLubyte) c;
			 infile>>c;  image6[i][j][1] = (GLubyte) c;
			 infile>>c;  image6[i][j][2] = (GLubyte) c;

		}
	}
	infile.close();


	// Initialize texture objects
    glGenTextures( 6, textures );


//------------------------------------- image 1 -------------------------------------
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, image1 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

//------------------------------------- image 2 -------------------------------------
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, image2 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
//------------------------------------- image 3 -------------------------------------
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, image3 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

//------------------------------------- image 4 -------------------------------------
    glBindTexture( GL_TEXTURE_2D, textures[3] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, image4 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
//------------------------------------- image 5 -------------------------------------
    glBindTexture( GL_TEXTURE_2D, textures[4] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, image5 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

//------------------------------------- image 6 -------------------------------------
    glBindTexture( GL_TEXTURE_2D, textures[5] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, image6 );
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
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(tex_coords),
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points)+sizeof(colors), sizeof(tex_coords), tex_coords );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader_cube_image.glsl", "fshader_cube_image.glsl" );
    glUseProgram( program );

    // set up vertex arrays
	GLintptr offset = 0; 
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(offset) );
	offset += sizeof(points);

    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(offset) );
	offset += sizeof(colors);

	GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(offset) );

	// Set the value of the fragment shader texture sampler variable
	//   ("texture") to the the appropriate texture unit. In this case,
    //   zero, for GL_TEXTURE0 which was previously set by calling
    //   glActiveTexture().
    glUniform1i( glGetUniformLocation(program, "texture"), 0 );
    ModelView_loc = glGetUniformLocation( program, "ModelView" );
    Projection_loc = glGetUniformLocation( program, "Projection" );

    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.8, 0.8, 0.8, 1.0 ); 
}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	
	//Generate the ModelView Matrix and send to vertex shader:
	mat4  mv = polarView(zoomin, twist, elevation, azimuth);	
    glUniformMatrix4fv( ModelView_loc, 1, GL_TRUE, mv );

	//Generate the Projection Matrix and send to vertex shader:
	p = Frustum( -1.0, 1.0, -1.0, 1.0, 1.0, 10.0 );
    glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, p );

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
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; tex_coords[Index] = vec2( 0.0, 1.0 );
	Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; tex_coords[Index] = vec2( 1.0, 0.0 );
	Index++;
}

//----------------------------------------------------------------------------

void colorcube()
{
    quad( 1, 0, 3, 2 );		//Front (Red)
    quad( 2, 3, 7, 6 );		//Right	(Yellow)
    quad( 3, 0, 4, 7 );		//Bottom (Green)
    quad( 6, 5, 1, 2 );		//Top  (White)
    quad( 4, 5, 6, 7 );		//Back	(Blue)
    quad( 5, 4, 0, 1 );		//Left (Magenta)
}

//----------------------------------------------------------------------------

mat4 polarView(GLdouble distance, GLdouble twist, GLdouble elevation, GLdouble azimuth)
{
	mat4 c;
	c = Translate(0.0, 0.0, -distance) * RotateZ(twist) * RotateX(elevation) * RotateY(-azimuth);
	return c; 
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
