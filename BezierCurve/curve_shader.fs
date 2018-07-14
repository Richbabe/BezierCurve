//曲线片段着色器源代码
#version 330 core
out vec4 FragColor;

uniform vec3 curveColor;//从顶点着色器传来的输入变量(名称相同、类型相同)

void main()
{
    FragColor = vec4(curveColor, 1.0f);
}