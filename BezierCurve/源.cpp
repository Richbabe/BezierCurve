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

int screenHeight = 600;//���ڸ߶�
int screenWidth = 800;//���ڿ��

bool isLeftClick = false;//�ж�����Ƿ������
int click_xpos;//�������x����
int click_ypos;//�������y����

bool isRightClick = false;//�ж�����Ƿ����Ҽ�

vector<pair<GLfloat, GLfloat> > Points;//ȷ��Bezier���ߵ��ĸ���

//��xת�����ӿ�����ϵ��x
GLfloat x_convert(int x) {
	return (x - screenWidth / 2.0) * 2.0 / screenWidth;
}

//��yת�����ӿ�����ϵ��y
GLfloat y_convert(int y) {
	return (screenHeight / 2.0 - y) * 2.0 / screenHeight;
}

//��ÿ�δ��ڴ�С��������ʱ����õĻص�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//�����·��ؼ�(ESC)��رմ���
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

//��ÿ������ƶ�ʱ���ûص�����
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	click_xpos = xpos;
	click_ypos = ypos;
}

//��������ʱ�Ļص�����
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

//���㺯��
void drawPoints(float fx, float fy, Shader shader) {
	float vertices[] = {
		fx, fy, 0.0f
	};
	unsigned int points_VBO;//���㻺�����
	unsigned int points_VAO;//�����������
	glGenVertexArrays(1, &points_VAO);//����һ��VAO����
	glGenBuffers(1, &points_VBO);//����һ��VBO����
	glBindVertexArray(points_VAO);//��VAO
	//�Ѷ������鸴�Ƶ������й�OpengGLʹ��
	glBindBuffer(GL_ARRAY_BUFFER, points_VBO);//���´����Ļ���VBO�󶨵�GL_ARRAY_BUFFERĿ����
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//��֮ǰ����Ķ�������points_vertices���Ƶ�������ڴ���

    //���Ӷ�������
	//λ�����ԣ�ֵΪ0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//������������
	glEnableVertexAttribArray(0);
	shader.use();//������ɫ���������
	glBindVertexArray(points_VAO);//��VAO
	glPointSize(5);
	glDrawArrays(GL_POINTS, 0, 1);//����ͼԪ
	//glBindVertexArray(0);
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);
}

//�����������ߺ���
void drawCurve(Shader shader) {
	for (GLfloat t = 0; t <= 1.0; t += 0.001) {
		//����������ÿ�����x�����y����
		GLfloat x = Points[0].first * pow(1.0f - t, 3) + 3 * Points[1].first * t * pow(1.0f - t, 2) 
			+ 3 * Points[2].first * t * t * (1.0f - t) + Points[3].first * pow(t, 3);
		GLfloat y = Points[0].second * pow(1.0f - t, 3) + 3 * Points[1].second * t * pow(1.0f - t, 2) 
			+ 3 * Points[2].second * t * t * (1.0f - t) + Points[3].second * pow(t, 3);
	
		//������
		drawPoints(x_convert(x), y_convert(y), shader);
	}
}

int main() {
	glfwInit();//��ʼ��GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//����GLFW���汾��Ϊ3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//����GLFW�ΰ汾��Ϊ3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ�ú���ģʽ
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);(MACϵͳ���ע��)

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "BezierCurve", NULL, NULL);//�������ڶ���
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1);

	//�����κ�OpenGL�ĺ���֮ǰ��Ҫ��ʼ��GLAD
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

	bool my_tool_active = true;//���ô����Ƿ���Ч

	glm::vec3 change_curve_color = glm::vec3(0.0f, 1.0f, 0.0f);//imGUI��ɫ�����ɵ�������ɫ

	glViewport(0, 0, screenWidth, screenHeight);

	Shader pointShader("shader.vs", "point_shader.fs");//�����Ķ�����ɫ����Ƭ����ɫ��������
	Shader curveShader("shader.vs", "curve_shader.fs");//�������ߵĶ�����ɫ����Ƭ����ɫ��������

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);//GLFWע��ص���������������mouse_button_callback�����ͻᱻ����
	glfwSetCursorPosCallback(window, mouse_callback);//GLFWע��ص������������һ�ƶ�mouse_callback�����ͻᱻ����

	
	//���GLFW��Ҫ���˳������ѭ��
	while (!glfwWindowShouldClose(window))
	{
		//����
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		//-----------------��Ⱦָ��-----------------
		static float f = 0.0f;

		//����˵���
		ImGui::Begin("BezierCurve", &my_tool_active, ImGuiWindowFlags_MenuBar);//��ʼGUI������Ⱦ
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Close", "Ctrl+W")) {
					my_tool_active = false;
					glfwSetWindowShouldClose(window, true);//�رմ���
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// GUI���ò���
		ImGui::ColorEdit3("curve color", (float*)&change_curve_color); // ����������ɫ
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
		ImGui::End();//����GUI������Ⱦ

		//��Ⱦ
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//���������Ļ���õ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);//�����Ļ����ɫ����

		//�����ӵ�����ƶ����λ��
		if (isLeftClick) {
			//cout << "LeftClick:" << click_xpos << " " << click_ypos << endl;
			//�������Ŀ����4��ӵ�
			if (Points.size() < 4) {
				Points.push_back(make_pair(GLfloat(click_xpos), GLfloat(click_ypos)));
			}
			//�������Ŀ���ڵ���4�ƶ����λ��
			else{
				for (int i = 0; i < Points.size(); i++) {
					//�϶��������ڡ�-50��50��
					if (Points[i].first <= click_xpos + 50 && Points[i].first >= click_xpos - 50 && Points[i].second <= click_ypos + 50 && Points[i].second >= click_ypos - 50) {
						//���µ��λ��
						Points[i].first = GLfloat(click_xpos);
						Points[i].second = GLfloat(click_ypos);
					}
				}
			}
			isLeftClick = false;
		}

		//�Ҽ�ȥ����
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

		//����
		for (int i = 0; i < Points.size(); i++) {
			//cout << Points[i].first << " " << Points[i].second << endl;
			drawPoints(x_convert(Points[i].first), y_convert(Points[i].second), pointShader);
		}
		//������
		if (Points.size() == 4) {
			drawCurve(curveShader);
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		//��鲢�����¼�����������
		glfwSwapBuffers(window);//������ɫ����
		glfwPollEvents();//�����û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬�������ö�Ӧ�Ļص�����������ͨ���ص������ֶ����ã���
	}
	//�ͷ���Դ;
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();//��ȷ�ͷ�/ɾ��֮ǰ�ķ����������Դ

	return 0;
}