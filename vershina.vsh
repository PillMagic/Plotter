#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif
attribute highp vec4 qt_Vertex;
attribute highp vec4 color;
uniform highp mat4 qt_ModelViewProjectionMatrix;
varying vec4 colorcolor;
void main(void)
{
     gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
     colorcolor = color;
}
