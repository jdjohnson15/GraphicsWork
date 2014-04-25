varying vec4 fColors1;
varying vec4 fColors2;
varying vec4 fColors3;

uniform float time;
uniform float dropTime;

void main()
{
	if (time > dropTime) 
		gl_FragColor = mix(fColors1, fColors2, time);
	if (time < dropTime)
		gl_FragColor = mix(fColors1, fColors3, time);
}
