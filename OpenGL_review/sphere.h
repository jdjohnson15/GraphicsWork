//Sphere.h
//Sphere group
//12/5/2013
//Carolyn Owens, Zachery Stanley, Brandon Hook, and Dylan Phillips.
// Header and implementation for a function to get the points, face normals, and vertex normals for a sphere.
// Sends back 3 arrays which will hold points, face normals, and vertex normals respectively. 
// Assumes Angel.h and all required files of his are in the same directory. 

//Important Note: Normals may be off and the sphere may not work correctly with lighting. 


#ifndef SPHERE_H
#define SPHERE_H

#include "Angel.h"

typedef Angel::vec4 point4;

//Pre-Conditions: Slices and stacks must be a non-negative whole number less than or equal to 361.
    //points[] and point_normal[] must have an array size equal to or greater than 
    //"(slices*stacks)*4." face_normal[] must have an array size equal to or greater than "(slices*stacks)."
//Post-Condition: For points[], sent back the specified number of points to create a sphere shape.
    //For face_normal[], sent back each face normal per 4 points. For vertex_normal, sent back specified
    //vertex normals for each point. 
    void Sphere (float radius, int slices, int stacks, vec4 points[], vec3 face_normal[], vec3 point_normal[])
    {
        
        vec4 points1[(slices*stacks)*5];
        float xPos= 0.0;
        float yPos= 0.0;
        float zPos= 0.0;
        float degValue=360.0/slices;
        float degValue2=(360.0/stacks)/2;
        int count=0;
        float debug;
        
        for (int a = 0; a<stacks+1; a++)
        {
            for (int b =0; b<slices; b++)
            {
                zPos = cos(DegreesToRadians*(a*degValue2)) * radius;
                xPos = cos(DegreesToRadians*(b*degValue))*sin(DegreesToRadians*(a*degValue2))*radius ;
                yPos = sin(DegreesToRadians*(b*degValue)) *sin(DegreesToRadians*(a*degValue2))*radius;
                


                
                points1[count] = point4(xPos, yPos, zPos, 1.0);
                count++;
            }
            
        }
        
        int j = 0;
        
        int q=0;
        for (int i = 0; i<count; i++){
            
            points[j] = points1[i];
            j++;
            points[j] = points1[i+1];
            j++;
            points[j] = points1[i+slices];
            j++;
            points[j] = points1[i+slices+1];
            j++;
            
            
           
            
            face_normal[i] = cross((points[j-3]-points[j-4]),(points[j-2]-points[i-4]));
            if (face_normal[i].x != -0 && face_normal[i].y != -0 && face_normal[i].z != -0)
            {
                face_normal[i] = normalize(face_normal[i]);
            }

            
                point_normal[q].x=points[q].x;
                point_normal[q].y=points[q].y;
                point_normal[q].z=points[q].z;
		point_normal[q]=normalize(point_normal[q]);
                q++;
                point_normal[q].x=points[q].x;
                point_normal[q].y=points[q].y;
                point_normal[q].z=points[q].z;
		point_normal[q]=normalize(point_normal[q]);
                q++;
                point_normal[q].x=points[q].x;
                point_normal[q].y=points[q].y;
                point_normal[q].z=points[q].z;
		point_normal[q]=normalize(point_normal[q]);
                q++;
                point_normal[q].x=points[q].x;
                point_normal[q].y=points[q].y;
                point_normal[q].z=points[q].z;
		point_normal[q]=normalize(point_normal[q]);
                q++;
            
        }
        
    }
    


#endif
