#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <memory>

#include "Shader.h"
#include "Object.h"
#include "Sphere.h"
#include "Camera.h"

const double TARGET_FPS = 240.0f;
const float PIXELS_PER_METER = 50.0f;

const float SCREEN_WIDTH = 1280.0f;
const float SCREEN_HEIGHT = 720.0f;

const float SIM_WIDTH = SCREEN_WIDTH / PIXELS_PER_METER;
const float SIM_HEIGHT = SCREEN_HEIGHT / PIXELS_PER_METER;
const float SIM_DEPTH = SIM_WIDTH;

GLFWwindow* startGLFW();
bool initGLAD();
void addRandomSpheres(std::vector<std::unique_ptr<Object>>& objects, unsigned int count);




int main()
{
    GLFWwindow* window = startGLFW();

    if (!window || !initGLAD()) return -1;

    Shader shaderProgram = [&]() {
        try {
            return Shader("glsl shaders/default.vert", "glsl shaders/default.frag");
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            glfwTerminate();
            std::exit(-1);
        }
    }();

    int colorLoc = glGetUniformLocation(shaderProgram.ID, "objectColor");
    int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");

    double lastTime = glfwGetTime();

    std::vector<std::unique_ptr<Object>> objects;
	// addRandomSpheres(objects, 100);

    

	// Centre the camera on the scene, pull back on Z so the full volume is visible
	Camera camera((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT,
		glm::vec3(SIM_WIDTH / 2.0f, SIM_HEIGHT / 2.0f, SIM_WIDTH));

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;

        if (deltaTime >= (1.0 / TARGET_FPS)) {
            float dt = static_cast<float>(deltaTime);
            lastTime = currentTime;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shaderProgram.Activate();

            ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

            if (!io.WantCaptureKeyboard) {
                camera.inputs(window);
                camera.matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");
            }

			for (auto& obj : objects) {
				obj->updatePos(dt);
				obj->boundaryCheck(SIM_WIDTH, SIM_HEIGHT, SIM_DEPTH);
				obj->draw(modelLoc, colorLoc);
			}

			ImGui::Begin("Simulation Controls");
            ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
			ImGui::Text("Object Count: %d", (int)objects.size());
			ImGui::End();

			ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        glfwPollEvents();
    }

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    objects.clear();
    shaderProgram.Delete();
    glfwTerminate();
    return 0;
}

GLFWwindow* startGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, "simulation", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    return window;
}

bool initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

void addRandomSpheres(std::vector<std::unique_ptr<Object>>& objects, unsigned int count) {
    for (int i = 0; i < count; i++) {
        float x = static_cast<float>(rand()) / RAND_MAX * SIM_WIDTH;
        float y = static_cast<float>(rand()) / RAND_MAX * SIM_HEIGHT;
        float z = (static_cast<float>(rand()) / RAND_MAX) * SIM_DEPTH;
        float vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
        float vy = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
        float vz = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
        float red = static_cast<float>(rand()) / RAND_MAX;
        float green = static_cast<float>(rand()) / RAND_MAX;
        float blue = static_cast<float>(rand()) / RAND_MAX;
        objects.push_back(std::make_unique<Sphere>(glm::vec3(x, y, z), glm::vec3(vx, vy, vz), 0.3f, glm::vec3(red, green, blue)));
    }
}