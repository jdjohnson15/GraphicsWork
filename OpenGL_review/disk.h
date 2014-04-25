// disk.h
// Tyler Arnold, Brandon Dreager, Toni Jackson, Jesse Johnson
// December 5, 2013
//
// Using outer and inner radii, number for both slices and rings, start and sweep angles (in degrees),
// it generates an array of vertices to create a disk and the normal for the disk's face.

#include <math.h> // Gives the cos and sin functions

// Precondition: outerRadius and innerRadius must be unique, positive integers
//					although outer does not need to be greater than inner.
//					numSlices must be > 1 and numRings > 0. Best results use more rings and slices.
//					startAng can be any integer, but sweepAng must be between 0 and 360.
//					The vec4 array points must be a constant size of numSlices*numRings*6 (for both triangles per square)
//					and normal needs to be a replacable vec3
// Postcondition: "points" was filled with points ready to display your disk
//					and the normal for the entire disk was stored in "normal"

void Disk (double outerRadius, double innerRadius, int numSlices, int numRings, double startAng, double sweepAng,
		   vec4 points[], vec3& normal)
{
	double slice_inc = sweepAng / numSlices;
	double ring_inc = (outerRadius- innerRadius) / numRings;
	int vIndex = 0;
	vec4 vertices[(numSlices+1)*(numRings+1)];

	// Create the vertices to make up the disk
	for (int i = 0; i < numRings+1; i++)
	{
		float currRing = i * ring_inc + innerRadius;
		for (int j = 0; j < numSlices+1; j++)
		{
			float currSlice = j * slice_inc + startAng;
			currSlice = currSlice * (M_PI / 180); // Turn it to radians
			vertices[vIndex].x = currRing * cos(currSlice);
			vertices[vIndex].y = currRing * sin(currSlice);
			vertices[vIndex].z = 0;
			vertices[vIndex].w = 1;
			++vIndex;
		}
	}

	// Put the vertices in proper order to be drawn
	int pIndex = 0;
	for (int i = 0; i < numRings; ++i)
	{
		int mod = i * numSlices + i;
		for (int j = 0; j < numSlices; ++j)
		{
			points[pIndex] = vertices[mod + j];
			pIndex++;
			points[pIndex] = vertices[mod + j+numSlices+1];
			pIndex++;
			points[pIndex] = vertices[mod + j+numSlices+2];
			pIndex++;
			
			points[pIndex] = vertices[mod + j];
			pIndex++;
			points[pIndex] = vertices[mod + j+1];
			pIndex++;
			points[pIndex] = vertices[mod + j+numSlices+2];
			pIndex++;
		}
	}
	
	// The normal... which is the same for all vertices and the one face
	normal.x = 0;
	normal.y = 0;
	normal.z = 1;

	return;
}
