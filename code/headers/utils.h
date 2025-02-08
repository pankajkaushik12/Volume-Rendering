#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

// Include glfw3.h after our OpenGL definitions
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define WIDTH 1920
#define HEIGHT 1080
#define WINDOWNAME "Volume Rendering"

struct TransferFunctionPoint {
	float position; // 0.0 to 1.0 (Normalized)
	ImVec4 color;   // RGBA
};

class GLFWindow {
private:
	int Width, Height;
	std::string WindowName;

	GLFWwindow* Window;

	GLuint ShaderProgram;

	int openGLInit();
	const char* setGLSLVersion();
	
	const char* vShaderFile = "../shaders/vshader11.fs";
	const char* fShaderFile = "../shaders/fshader11.fs";
	char* getShaderCode(const char* filename);
	GLuint createShader(const char* filename, GLenum type);
	
	void printLog(GLuint object);

	float step_size = 1;
	glm::vec3 VolumeSize;

	GLint vModel_uniform, vView_uniform, vProjection_uniform;
	GLint vColor_uniform;

	GLuint VAO, tfTex, volumeTex;
	GLfloat* TransferFun = new GLfloat[256 * 4];
	int selectedControlPoint = 0;

	ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

	std::vector<TransferFunctionPoint> controlPoints = {
		{0.0f, ImVec4(0, 0, 0, 0)},
		{1.0f, ImVec4(1, 1, 1, 1)}
	};

public:
	GLFWindow(int width = WIDTH, int height = HEIGHT, std::string name = WINDOWNAME);
	GLFWwindow* setupWindow();

	bool renderParamsChanged = false;
	float currentFrameTime = 0.0f, lastFrameTime = 0.0f, deltaTime = 0.0f;
	glm::vec3 camposition = glm::vec3(0, 0, 280.0);
	glm::vec3 camup = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 camat = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 camright = glm::vec3(1, 0, 0);
	glm::mat4 modelT, viewT, projectionT;//The model, view and projection transformations

	virtual void key(int key, int action, int mods) {}
	virtual void mouseMotion(float xPos, float yPos) {}
	virtual void mouseScroll(float xOffset, float yOffset) {}
	virtual void mouseButton(int button, int action, int mods) {}

	unsigned int CreateShaderProgram(const char* vShader_filename, const char* fShader_filename);

	void DrawTransferFunctionEditor();
	void RenderGUI();

	void SetUniforms();

	void SetupViewTransformation();
	void SetupModelTransformation();
	void SetupProjectionTransformation();
	glm::vec3 getTrackBallVector(double x, double y);

	void Create3DVolumeTexture(GLubyte*, float x_size, float y_size, float z_size);
	void Create1DTransferFunction();

	void CreateBoundingBox();

	bool Run();

	void ResizeWindow(int width, int height);

	void cleanup();
};

struct GLFCameraWindow : public GLFWindow {
	GLFCameraWindow(int width, int height, std::string title)
		: GLFWindow(width, height, title)
	{
		glm::vec3 camera_from = camposition;
		glm::vec3 camera_at = glm::vec3(0, 0, 0);
		glm::vec3 camera_up = glm::vec3(0, 1, 0);

		camposition = camera_from;
		camup = camera_up;
		camfront = glm::normalize(camera_at - camera_from);
		camright = glm::normalize(glm::cross(camfront, camup));
		camat = camposition + camfront * 10.f;
		SetupViewTransformation();
	}

	void updateFrontVector(glm::vec3 front_)
	{
		camfront = front_;
		camright = glm::normalize(glm::cross(camfront, camup));
		camup = glm::normalize(glm::cross(camright, camfront));
		camat = camposition + camfront * 10.f;
	}

	// Virtual function to handle keyboard input
	virtual void key(int key, int action, int mods) override
	{
		float velocity = speed * deltaTime;

		switch (key) {
		case GLFW_KEY_W:
			camposition += camfront * velocity;
			break;
		case GLFW_KEY_S:
			camposition -= camfront * velocity;
			break;
		case GLFW_KEY_A:
			camposition -= glm::normalize(glm::cross(camfront, camup)) * velocity;
			break;
		case GLFW_KEY_D:
			camposition += glm::normalize(glm::cross(camfront, camup)) * velocity;
			break;
		case GLFW_KEY_Q:
			camposition -= camup * velocity;
			break;
		case GLFW_KEY_E:
			camposition += camup * velocity;
			break;
		}

		SetupViewTransformation();

		renderParamsChanged = true;
	}

	virtual void mouseMotion(float xPos, float yPos) override
	{
		if (RightButtonPressed)
		{
			if (firstMouseMovement) {
				lastX = xPos;
				lastY = yPos;
				firstMouseMovement = false;
				return;
			}

			float xoffset = xPos - lastX;
			float yoffset = lastY - yPos; // Reversed
			lastX = xPos, lastY = yPos;

			yaw += xoffset * sensitivity;
			pitch += yoffset * sensitivity;

			// Clamp pitch
			pitch = glm::clamp(pitch, -89.0f, 89.0f);

			// Calculate new front vector
			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			camfront = glm::normalize(direction);

			updateFrontVector(camfront);

			SetupViewTransformation();

			renderParamsChanged = true;
		}
		if (LeftButtonPressed) {
			if (firstMouseMovement) {
				lastX = xPos;
				lastY = yPos;
				firstMouseMovement = false;
				return;
			}

			glm::vec3 va = getTrackBallVector(lastX, lastY);
			glm::vec3 vb = getTrackBallVector(xPos, yPos);
			float angle = acos(std::min(1.0f, glm::dot(va, vb)));
			glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
			glm::mat3 camera2object = glm::inverse(glm::mat3(viewT * modelT));
			glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
			glm::mat4 dummy = glm::rotate(modelT, -angle, axis_in_object_coord);
			camposition = glm::vec4(glm::mat3(dummy) * glm::vec3(camposition), 1.0);
			camat = glm::vec3(0, 0, 0);
			camfront = glm::normalize(camat - camposition);
			camup = glm::vec3(0, 1, 0);
			SetupViewTransformation();

			lastX = xPos;
			lastY = yPos;
		}
	}

	virtual void mouseScroll(float xoffset, float yoffset)
	{
		if (!RightButtonPressed) return;

		speed = yoffset > 0 ? speed * 2.f : speed / 2.f;
		speed = glm::clamp(speed, 128.0f, 4096.0f);
	}

	virtual void mouseButton(int button, int action, int mods) override
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				RightButtonPressed = true;
				firstMouseMovement = true;
			}
			else if (action == GLFW_RELEASE) {
				RightButtonPressed = false;
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				LeftButtonPressed = true;
				firstMouseMovement = true;
			}
			else if (action == GLFW_RELEASE) {
				LeftButtonPressed = false;
			}
		}
	}

	int keyPressed;
	/*
  * Mouse input variables
  */
	bool RightButtonPressed = false;
	bool LeftButtonPressed = false;
	bool firstMouseMovement = true;
	float lastX, lastY;
	float speed = 128.f, sensitivity = 0.1f;
	float yaw = -90.f, pitch = 0.f;
	glm::vec3 camfront = glm::vec3(0, 0, -1);
};