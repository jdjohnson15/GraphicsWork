// FILE: mymips.cpp
// NAME: J. Johnson
// Winter, 2014


//	 	Demonstrates using user-supplied mipmaps for improved texture mapping.  To verify, we
//		are using three different images.  They are slipped in for level 2 and level 4.
//
//  	Uses 
//	-	NEAREST min filter and
//	-	LINEAR mipmap
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
GLuint textures[1];
GLubyte image256[256][256][4];
GLubyte image128[128][128][4];
GLubyte image64 [64][64][4];
GLubyte image32 [32][32][4];
GLubyte image16 [16][16][4];
GLubyte image8  [8][8][4];
GLubyte image4  [4][4][4];
GLubyte image2  [2][2][4];

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

//void loadImage(int tex_size, string file_name, GLubyte image);
// Precondition:  tex_size is the size of the image being used, in pixels.
//		  file_name is a .ppm file in the current directory that has the first 4 lines of the code from .ppm file removed.
//		  image is the moniker the image will have in the program.
// Postcondition: the given image file's color data is saved into the array of images used for textures. 

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
    glutCreateWindow( "Toa Nuva Cube (with manual mipmapping)" );
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


	int i, j, c;

//----------------------------- 256 image --------------------------------------
	ifstream infile;
	infile.open("1_256.ppm");
	for (j = 0; j < 256; j++) 
	{
		for (i = 0; i < 256; i++) 
		{
			infile>>c;  image256[i][j][0] = (GLubyte) c;
			infile>>c;  image256[i][j][1] = (GLubyte) c;
			infile>>c;  image256[i][j][2] = (GLubyte) c;
		        image256[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();
//----------------------------- 128 image (blue) --------------------------------------
	infile.open("1_128.ppm");
	for (j = 0; j < 128; j++) 
	{
		for (i = 0; i < 128; i++) 
		{
			infile>>c;  image128[i][j][0] = (GLubyte) c;
			infile>>c;  image128[i][j][1] = (GLubyte) c;
			infile>>c;  image128[i][j][2] = (GLubyte) c;
		        image128[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();

//----------------------------- 64 image ---------------------------------------
	infile.open("1_64.ppm");
	for (j = 0; j < 64; j++) 
	{
		for (i = 0; i < 64; i++) 
		{
			infile>>c;  image64[i][j][0] = (GLubyte) c;
			infile>>c;  image64[i][j][1] = (GLubyte) c;
			infile>>c;  image64[i][j][2] = (GLubyte) c;
		        image64[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();

//----------------------------- 32 image (green)---------------------------------------
	infile.open("1_32.ppm");
	for (j = 0; j < 32; j++) 
	{
		for (i = 0; i < 32; i++) 
		{
			infile>>c;  image32[i][j][0] = (GLubyte) c;
			infile>>c;  image32[i][j][1] = (GLubyte) c;
			infile>>c;  image32[i][j][2] = (GLubyte) c;
		        image32[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();

//----------------------------- 16 image ---------------------------------------
	infile.open("1_16.ppm");
	for (j = 0; j < 16; j++) 
	{
		for (i = 0; i < 16; i++) 
		{
			infile>>c;  image16[i][j][0] = (GLubyte) c;
			infile>>c;  image16[i][j][1] = (GLubyte) c;
			infile>>c;  image16[i][j][2] = (GLubyte) c;
		        image16[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();

//----------------------------- 8 image ----------------------------------------
	infile.open("1_8.ppm");
	for (j = 0; j < 8; j++) 
	{
		for (i = 0; i < 8; i++) 
		{
			infile>>c;  image8[i][j][0] = (GLubyte) c;
			infile>>c;  image8[i][j][1] = (GLubyte) c;
			infile>>c;  image8[i][j][2] = (GLubyte) c;
		        image8[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();

//----------------------------- 4 image ----------------------------------------
	infile.open("1_4.ppm");
	for (j = 0; j < 4; j++) 
	{
		for (i = 0; i < 4; i++) 
		{
			infile>>c;  image4[i][j][0] = (GLubyte) c;
			infile>>c;  image4[i][j][1] = (GLubyte) c;
			infile>>c;  image4[i][j][2] = (GLubyte) c;
		        image4[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();

//----------------------------- 2 image ----------------------------------------
	infile.open("1_2.ppm");
	for (j = 0; j < 2; j++) 
	{
		for (i = 0; i < 2; i++) 
		{
			infile>>c;  image2[i][j][0] = (GLubyte) c;
			infile>>c;  image2[i][j][1] = (GLubyte) c;
			infile>>c;  image2[i][j][2] = (GLubyte) c;
		        image2[i][j][3] = (GLubyte) 255;
		}
	}
	infile.close();



	// Initialize texture objects
        glGenTextures( 2, textures );

	glBindTexture( GL_TEXTURE_2D, textures[0] );

	//Mipmapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 7);
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );

	//Specify the data for the textures
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image256);
	glTexImage2D(GL_TEXTURE_2D, 1, 4, 128, 128, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image128);
	glTexImage2D(GL_TEXTURE_2D, 2, 4, 64, 64, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image64);
	glTexImage2D(GL_TEXTURE_2D, 3, 4, 32, 32, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	glTexImage2D(GL_TEXTURE_2D, 4, 4, 16, 16, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image16);
	glTexImage2D(GL_TEXTURE_2D, 5, 4, 8, 8, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image8);
	glTexImage2D(GL_TEXTURE_2D, 6, 4, 4, 4, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image4);
	glTexImage2D(GL_TEXTURE_2D, 7, 4, 2, 2, 
		        0, GL_RGBA, GL_UNSIGNED_BYTE, image2);



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
    glDrawArrays( GL_TRIANGLES, 0, 36 );


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

