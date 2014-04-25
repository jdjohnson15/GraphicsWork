
attribute vec4 vColor;
attribute vec4 vPosition;

varying vec4 color;


uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main() 
{
  gl_Position = Projection * View * Model * vPosition;
  color = vColor;
} 

	








