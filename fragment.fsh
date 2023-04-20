#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif
varying vec4 colorcolor;
void main(void)
{
    gl_FragColor = colorcolor;
}
