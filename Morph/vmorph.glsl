attribute vec4 vertices1;
attribute vec4 vertices2;
attribute vec4 vertices3;
attribute vec4 vColors1;
attribute vec4 vColors2;
attribute vec4 vColors3;

varying   vec4 fColors1;
varying   vec4 fColors2;
varying   vec4 fColors3;


uniform float time;
uniform float dropTime;

bool change = false;

void main()
{
	if (time > 0.9) 
		change = true;
	if (time < 0.3)
		change = false;

	if (change = false) 
		gl_Position = mix(vertices1, vertices2, time);
	if (change = true)
		gl_Position = mix(vertices3, vertices2, time);


	fColors1 = vColors1;
	fColors2 = vColors2;
	fColors3 = vColors3;
}
