#version 300 es
// Shader for simple textured geometry.

precision mediump float;
uniform mat4 uMvpMatrix;//by DALi
uniform vec3 uSize;  // by DALi
in vec3 aPosition;

void main()
{
  gl_Position = uMvpMatrix * vec4(aPosition * uSize, 1.f);
}
