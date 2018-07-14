#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_s.h>

using namespace std;

int screenHeight = 600;//窗口高度
int screenWidth = 800;//窗口宽度

bool isLeftClick = false;//判断鼠标是否点击左键
int click_xpos;//鼠标点击的x坐标
int click_ypos;//鼠标点击的y坐标

bool isRightClick = false;//判断鼠标是否点击右键

vector<pair<GLfloat, GLfloat> > Points;//确定Bezier曲线的四个点

//将x转换成视口坐标系的x
GLfloat x_convert(int x) {
	return (x - screenWidth / 2.0) * 2.0 / screenWidth;
}

//将y转换成视口坐标系的y
GLfloat y_convert(int y) {
	return (screenHeight / 2.0 - y) * 2.0 / screenHeight;
}

//在每次窗口大小被调整的时候调用的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//若按下返回键(ESC)则关闭窗口
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

//在每次鼠标移动时调用回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	click_xpos = xpos;
	click_ypos = ypos;
}

//当点击鼠标时的回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		isLeftClick = true;
		//cout << "leftClick" << endl;
	}
	else if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
		isRightClick = true;
		//cout << "rightClick" << endl;
	}
	return;
}

//画点函数
void drawPoints(float fx, float fy, Shader shader) {
	float vertices[] = {
		fx, fy, 0.0f
	};
	unsigned int points_VBO;//顶点缓冲对象
	unsigned int points_VAO;//顶点数组对象
	glGenVertexArrays(1, &points_VAO);//生成一个VAO对象
	glGenBuffers(1, &points_VBO);//生成一个VBO对象
	glBindVertexArray(points_VAO);//绑定VAO
	//把顶点数组复制到缓冲中供OpengGL使用
	glBindBuffer(GL_ARRAY_BUFFER, points_VBO);//把新创建的缓冲VBO绑定到GL_ARRAY_BUFFER目标上
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//把之前定义的顶点数据points_vertices复制到缓冲的内存中

    //链接顶点属性
	//位置属性，值为0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//解析顶点数据
	glEnableVertexAttribArray(0);
	shader.use();//激活着色器程序对象
	glBindVertexArray(points_VAO);//绑定VAO
	glPointSize(5);
	glDrawArrays(GL_POINTS, 0, 1);//绘制图元
	//glBindVertexArray(0);
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);
}

//画贝塞尔曲线函数
void drawCurve(Shader shader) {
	for (GLfloat t = 0; t <= 1.0; t += 0.001) {
		//计算曲线上每个点的x坐标和y坐标
		GLfloat x = Points[0].first * pow(1.0f - t, 3) + 3 * Points[1].first * t * pow(1.0f - t, 2) 
			+ 3 * Points[2].first * t * t * (1.0f - t) + Points[3].first * pow(t, 3);
		GLfloat y = Points[0].second * pow(1.0f - t, 3) + 3 * Points[1].second * t * pow(1.0f - t, 2) 
			+ 3 * Points[2].second * t * t * (1.0f - t) + Points[3].second * pow(t, 3);
	
		//画曲线
		drawPoints(x_convert(x), y_convert(y), shader);
	}
}

int main() {
	glfwInit();//初始化GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置GLFW主版本号为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置GLFW次版本号为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心模式
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);(MAC系统解除注释)

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "BezierCurve", NULL, NULL);//声明窗口对象
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1);

	//调用任何OpenGL的函数之前需要初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	bool my_tool_active = true;//设置窗口是否有效

	glm::vec3 change_curve_color = glm::vec3(0.0f, 1.0f, 0.0f);//imGUI调色板生成的曲线颜色

	glViewport(0, 0, screenWidth, screenHeight);

	Shader pointShader("shader.vs", "point_shader.fs");//构造点的顶点着色器和片段着色器并链接
	Shader curveShader("shader.vs", "curve_shader.fs");//构造曲线的顶点着色器和片段着色器并链接

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);//GLFW注册回调函数，当点击鼠标mouse_button_callback函数就会被调用
	glfwSetCursorPosCallback(window, mouse_callback);//GLFW注册回调函数，当鼠标一移动mouse_callback函数就会被调用

	
	//如果GLFW被要求退出则结束循环
	while (!glfwWindowShouldClose(window))
	{
		//输入
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		//-----------------渲染指令-----------------
		static float f = 0.0f;

		//定义菜单栏
		ImGui::Begin("BezierCurve", &my_tool_active, ImGuiWindowFlags_MenuBar);//开始GUI窗口渲染
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Close", "Ctrl+W")) {
					my_tool_active = false;
					glfwSetWindowShouldClose(window, true);//关闭窗口
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// GUI设置参数
		ImGui::ColorEdit3("curve color", (float*)&change_curve_color); // 设置曲线颜色
		curveShader.setVec3("curveColor", change_curve_color);

		// Display contents in a scrolling region
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Created By Richbabe.");
		ImGui::BeginChild("Scrolling");
		/*
		for (int n = 0; n < 50; n++)
		ImGui::Text("%04d: Some text", n);
		*/
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::EndChild();
		ImGui::End();//结束GUI窗口渲染

		//渲染
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清空屏幕所用的颜色
		glClear(GL_COLOR_BUFFER_BIT);//清空屏幕的颜色缓冲

		//左键添加点或者移动点的位置
		if (isLeftClick) {
			//cout << "LeftClick:" << click_xpos << " " << click_ypos << endl;
			//如果点数目少于4添加点
			if (Points.size() < 4) {
				Points.push_back(make_pair(GLfloat(click_xpos), GLfloat(click_ypos)));
			}
			//如果点数目大于等于4移动点的位置
			else{
				for (int i = 0; i < Points.size(); i++) {
					//拖动作用域在【-50，50】
					if (Points[i].first <= click_xpos + 50 && Points[i].first >= click_xpos - 50 && Points[i].second <= click_ypos + 50 && Points[i].second >= click_ypos - 50) {
						//更新点的位置
						Points[i].first = GLfloat(click_xpos);
						Points[i].second = GLfloat(click_ypos);
					}
				}
			}
			isLeftClick = false;
		}

		//右键去除点
		if (isRightClick) {
			//cout << "rightClick:" << click_xpos << " " << click_ypos << endl;
			if (Points.size() > 0) {
				for (int i = 0; i < Points.size(); i++) {
					if (Points[i].first <= click_xpos + 3 && Points[i].first >= click_xpos - 3 && Points[i].second <= click_ypos + 3 && Points[i].second >= click_ypos - 3) {
						Points.erase(Points.begin() + i);
						//cout << "delete!" << endl;
					}
				}
			}
			isRightClick = false;
		}

		//画点
		for (int i = 0; i < Points.size(); i++) {
			//cout << Points[i].first << " " << Points[i].second << endl;
			drawPoints(x_convert(Points[i].first), y_convert(Points[i].second), pointShader);
		}
		//画曲线
		if (Points.size() == 4) {
			drawCurve(curveShader);
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		//检查并调用事件、交换缓冲
		glfwSwapBuffers(window);//交换颜色缓冲
		glfwPollEvents();//检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数（可以通过回调方法手动设置）。
	}
	//释放资源;
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();//正确释放/删除之前的分配的所有资源

	return 0;
}