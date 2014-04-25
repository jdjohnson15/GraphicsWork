attribute  vec4 vPosition;
attribute  vec4 vColor;
varying vec4 color;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main() 
{
    gl_Position = Projection*View*Model*vPosition;
    color = vColor;
}