//  FILE:  cylinder.h
//  NAME:  Huu Nguyen, Cody Jodrey, Brandon Fain, Jantzen Latham
//  DATE:  Dec. 4, 2013
//
//  DESCRIPTION:  Provides functions to generate the points and normal
//  vectors for a cylinder.
        
#include "Angel.h" 
#include <math.h> // Provides cos() and sin()

// Face structure holds four points p1, p2, p3, and p4
// for each quadrilateral of a cylinder and four normal
// vectors p1_normal, p2_normal, p3_normal, and p4_normal
// for the point normals of the four aforementioned points
// for each quadrilateral of a cylinder.
struct face {
   vec4 p1;
   vec4 p2;
   vec4 p3;
   vec4 p4;
   vec4 p1_normal;
   vec4 p2_normal;
   vec4 p3_normal;
   vec4 p4_normal;
};

////    FUNCTION DECLARATIONS    ////

//    quad
//
//    Preconditions  : a, b, c, and d are the vertices of a quadrilateral.  
//face_normal is the normal vector of the quadrilateral defined by these 
//points.  Index is a reference to the next available place in points[] which
//is an empty or partially filled array of points with at least 6 available 
//spots.  pt_normal_array[] is an empty or partially filled array of normal
//vectors at the same points as points[], also with at least 6 available spots.
//
//    Postconditions  : The correct 6 vertices (two repeated) have been
//loaded into points such such rendering them to the screen with triangles
//will construct a quadrilateral with the points a, b, c, and d.  face_normal
//has been added to every position in pt_normal_array[] to which a point was 
//added.  Index has increased by 6.          
void quad( vec4 a, vec4 b, vec4 c, vec4 d, vec4 face_normal, int& Index, vec4 points[], vec4 pt_normal_array[]);

//    Cylinder 
//
//    Preconditions  : baseRadius is the radius of the circle
//at the end of the cylinder located in the xy plane at z=0.  topRadius
//is the radius of the circle at the end of the cylinder in the xy plane
//at z=height.  1 and only one of baseRadius or topRadius may be 0 to 
//generate a cone.  height is the length of the cylinder.  slices and stacks
//determine the number of quadrilaterals used to approximate the cylinder.
//slices are divisions through the length of the cylinder, stacks are 
//divisions through the width of the cylinder.  
//
//pt_array[] and pt_normal_array are at least size stacks*slices*6.
//face_array[] is of size at least stacks*slices.
//
//    Postconditions : The points of a cylinder have been loaded into
//pt_array[] in the correct order to be rendered to the screen by means
//of triangles.  stacks*slices*6 points have been loaded.  The normal
//vectors of the faces of the cylinder have been loaded into face_array[].
//stacks*slices vectors have been loaded.  The normal vectors of the points
//of the cylinder have been loaded into pt_normal_array[] in the same order
//as in pt_array[] (ie, pt_normal_array[5] is the normal vector for pt_array[5]).
//stacks*slices*6 vectors have been loaded.
void Cylinder(float baseRadius, float topRadius, float height, int slices, int stacks, vec4 pt_array[], vec4 face_array[], vec4 pt_normal_array[]);


////    FUNCTION DEFINITIONS    ////

void quad( vec4 a, vec4 b, vec4 c, vec4 d, vec4 face_normal, int& Index, vec4 points[], vec4 pt_normal_array[])
{
    points[Index] = a; pt_normal_array[Index] += face_normal; Index++;
    points[Index] = b; pt_normal_array[Index] += face_normal; Index++;
    points[Index] = c; pt_normal_array[Index] += face_normal; Index++;
    points[Index] = a; pt_normal_array[Index] += face_normal; Index++;
    points[Index] = c; pt_normal_array[Index] += face_normal; Index++;
    points[Index] = d; pt_normal_array[Index] += face_normal; Index++;
}


void Cylinder(float baseRadius, float topRadius, float height, int slices, int stacks, vec4 pt_array[], vec4 face_array[], vec4 pt_normal_array[]) {
    float w, z, x, y;
    float Pi = 3.14159265359;
    int index = 0;
    int Index = 0;
    int numPoints = (stacks+1)*slices;
    vec4 points[numPoints];
    int quads = slices*stacks;
    face faces[quads]; 
    vec3 point_normals[numPoints];
    vec3 face_normal;
    
    //Generating points

    for (float z=0; z <= height; z=z+(height/stacks)) {
        for (float theta = 0; theta < 2*Pi; theta=theta+(2*Pi/slices)) {
           x = ((((topRadius-baseRadius)/height)*z)+baseRadius)*cos(theta);
           y = ((((topRadius-baseRadius)/height)*z)+baseRadius)*sin(theta);
           z = z;
           w = 1.0;
           points[index] = vec4(x,y,z,w);
           index++;


        }
    }
    //Generating faces


    for (int i = 0; i < quads; i++) {
        faces[i].p1 = points[i];
        if ((i+1)%slices != 0) {
                faces[i].p2 = points[i+1];
                faces[i].p4 = points[i+slices+1];
        }
        else {
                faces[i].p2 = points[(i+1)-slices];
                faces[i].p4 = points[i+1];              
        }
        faces[i].p3 = points[i+slices];

        //Normals
        face_normal = normalize(cross((faces[i].p2-faces[i].p1),(faces[i].p3-faces[i].p1)));
        face_array[i] = face_normal;
        
        quad(faces[i].p2, faces[i].p1, faces[i].p3, faces[i].p4, face_normal, Index, pt_array, pt_normal_array);
    }
    
    //Normalizing point normals
    for (int i = 0; i<sizeof(pt_normal_array); i++) {
        pt_normal_array[i] = normalize(pt_normal_array[i]);
    }
}

