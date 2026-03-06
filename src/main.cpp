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
#include "Grid.h"
#include "OrbitTrail.h"
#include "Scenarios.h"

// ---------- constants ----------
const double TARGET_FPS    = 240.0;
const float  SCREEN_WIDTH  = 1800.0f;
const float  SCREEN_HEIGHT = 1000.0f;
const float  G_SIM         = 4.0f * static_cast<float>(M_PI * M_PI);

GLFWwindow* startGLFW();
bool initGLAD();

// ---------- N-body acceleration ----------
static std::vector<glm::vec3> computeAccelerations(
	const std::vector<std::unique_ptr<Object>>& objects) {
	size_t n = objects.size();
	std::vector<glm::vec3> accels(n, glm::vec3(0.0f));
	for (size_t i = 0; i < n; i++) {
		for (size_t j = i + 1; j < n; j++) {
			glm::vec3 dir = objects[j]->pos - objects[i]->pos;
			float dist2 = glm::dot(dir, dir);
			float dist  = sqrtf(dist2);
			if (dist < 1e-6f) continue;
			float invDist3 = 1.0f / (dist * dist2);
			accels[i] += G_SIM * objects[j]->mass * dir * invDist3;
			accels[j] -= G_SIM * objects[i]->mass * dir * invDist3;
		}
	}
	return accels;
}

// ---------- rebuild trails for current objects ----------
static void rebuildTrails(std::vector<OrbitTrail>& trails,
	const std::vector<std::unique_ptr<Object>>& objects, int trailLength) {
	trails.clear();
	for (size_t i = 0; i < objects.size(); i++) {
		trails.emplace_back(trailLength, objects[i]->color * 0.6f);
	}
}

// ---------- main ----------
int main()
{
	GLFWwindow* window = startGLFW();
	if (!window || !initGLAD()) return -1;

	// ---------- shaders ----------
	auto loadShader = [](const char* vert, const char* frag) -> Shader {
		try { return Shader(vert, frag); }
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
			glfwTerminate();
			std::exit(-1);
		}
	};
	Shader bodyShader = loadShader("glsl shaders/default.vert", "glsl shaders/default.frag");
	Shader gridShader = loadShader("glsl shaders/grid.vert",    "glsl shaders/grid.frag");
	Shader glowShader = loadShader("glsl shaders/glow.vert",    "glsl shaders/glow.frag");

	// Body shader uniforms
	int modelLoc   = glGetUniformLocation(bodyShader.ID, "model");
	int colorLoc   = glGetUniformLocation(bodyShader.ID, "objectColor");
	int isStarLoc  = glGetUniformLocation(bodyShader.ID, "isStar");
	int starPosLoc = glGetUniformLocation(bodyShader.ID, "starPos");

	// Grid shader uniforms
	int gridCamLoc   = glGetUniformLocation(gridShader.ID, "camMatrix");
	int gridColorLoc = glGetUniformLocation(gridShader.ID, "lineColor");
	int gridBaseYLoc = glGetUniformLocation(gridShader.ID, "baseY");

	// Glow shader uniforms
	int glowCamLoc    = glGetUniformLocation(glowShader.ID, "camMatrix");
	int glowCenterLoc = glGetUniformLocation(glowShader.ID, "billboardCenter");
	int glowSizeLoc   = glGetUniformLocation(glowShader.ID, "billboardSize");
	int glowRightLoc  = glGetUniformLocation(glowShader.ID, "camRight");
	int glowUpLoc     = glGetUniformLocation(glowShader.ID, "camUp");
	int glowColorLoc  = glGetUniformLocation(glowShader.ID, "glowColor");

	// ---------- glow billboard quad ----------
	std::vector<float> quadVerts = { -1,-1,0, 1,-1,0, 1,1,0, -1,1,0 };
	std::vector<unsigned int> quadIdx = { 0,1,2, 0,2,3 };

	VAO glowVAO;
	glowVAO.Bind();
	VBO glowVBO(quadVerts);
	glowVAO.LinkAttrib(glowVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	EBO glowEBO(quadIdx);
	glowVAO.Unbind();
	glowVBO.Unbind();
	glowEBO.Unbind();

	// ---------- scenario & bodies ----------
	int currentScenario = static_cast<int>(ScenarioType::SolarSystem);
	int lastScenario    = -1;

	std::vector<std::unique_ptr<Object>> objects;
	std::vector<OrbitTrail> trails;
	std::vector<glm::vec3>  prevAccels;

	int trailLength = 200;

	// ---------- spacetime grid ----------
	float gridBaseY       = 1.0f;
	float curvatureFactor = 0.5f;
	float gridSize        = 12.0f;
	int   gridResolution  = 50;
	Grid  grid(gridResolution, gridSize, gridBaseY, curvatureFactor);
	bool  showGrid = true;

	// ---------- camera ----------
	Camera camera((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT,
		glm::vec3(0.0f, 8.0f, 14.0f));

	// Store camera pointer so the C-style scroll callback can reach it
	glfwSetWindowUserPointer(window, &camera);
	glfwSetScrollCallback(window, [](GLFWwindow* w, double, double yoff) {
		Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(w));
		if (cam) cam->scrollDelta += static_cast<float>(yoff);
	});

	// ---------- ImGui ----------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// ---------- sim state ----------
	float timeScale    = 0.5f;
	float visualScale  = 1.0f;
	int   trailStride  = 0;
	int   trailEvery   = 4;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		double deltaTime   = currentTime - lastTime;
		if (deltaTime < (1.0 / TARGET_FPS)) {
			glfwPollEvents();
			continue;
		}
		lastTime = currentTime;
		float dtReal = static_cast<float>(deltaTime);
		float dtSim  = dtReal * timeScale;

		// ---------- scenario switch ----------
		if (currentScenario != lastScenario) {
			loadScenario(static_cast<ScenarioType>(currentScenario), objects);
			rebuildTrails(trails, objects, trailLength);
			prevAccels = computeAccelerations(objects);
			trailStride = 0;
			lastScenario = currentScenario;
		}

		// ---------- Velocity Verlet integration ----------
		if (dtSim > 0.0f && !objects.empty()) {
			size_t n = objects.size();

			// Half-kick
			for (size_t i = 0; i < n; i++)
				objects[i]->vel += 0.5f * prevAccels[i] * dtSim;

			// Drift
			for (size_t i = 0; i < n; i++)
				objects[i]->updatePos(dtSim);

			// New accelerations
			std::vector<glm::vec3> newAccels = computeAccelerations(objects);

			// Half-kick with new accelerations
			for (size_t i = 0; i < n; i++)
				objects[i]->vel += 0.5f * newAccels[i] * dtSim;

			prevAccels = std::move(newAccels);
		}

		// ---------- orbit trail recording ----------
		trailStride++;
		if (trailStride >= trailEvery) {
			trailStride = 0;
			for (size_t i = 0; i < trails.size() && i < objects.size(); i++)
				trails[i].addPoint(objects[i]->pos);
		}

		// ---------- rendering ----------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
			camera.inputs(window);

		glm::vec3 camRight = glm::normalize(glm::cross(camera.Orientation, camera.Up));
		glm::vec3 camUp    = camera.Up;

		glm::mat4 view   = glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up);
		glm::mat4 proj   = glm::perspective(glm::radians(45.0f), SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 200.0f);
		glm::mat4 camMat = proj * view;

		// --- draw bodies ---
		bodyShader.Activate();
		camera.matrix(45.0f, 0.01f, 200.0f, bodyShader, "camMatrix");
		if (!objects.empty())
			glUniform3f(starPosLoc, objects[0]->pos.x, objects[0]->pos.y, objects[0]->pos.z);

		for (auto& obj : objects)
			obj->draw(modelLoc, colorLoc, isStarLoc, visualScale);

		// --- draw star glow (for every star) ---
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glowShader.Activate();
		glUniformMatrix4fv(glowCamLoc, 1, GL_FALSE, glm::value_ptr(camMat));
		glUniform3fv(glowRightLoc, 1, glm::value_ptr(camRight));
		glUniform3fv(glowUpLoc, 1, glm::value_ptr(camUp));

		for (auto& obj : objects) {
			Sphere* s = dynamic_cast<Sphere*>(obj.get());
			if (s && s->isStar) {
				glUniform3fv(glowCenterLoc, 1, glm::value_ptr(s->pos));
				glUniform1f(glowSizeLoc, s->r * visualScale * 4.8f);
				glUniform4f(glowColorLoc, s->color.r, s->color.g, s->color.b * 0.8f, 0.9f);
				glowVAO.Bind();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glowVAO.Unbind();
			}
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);

		// --- draw spacetime grid ---
		if (showGrid) {
			grid.curvatureFactor = curvatureFactor;
			grid.halfSize = gridSize;
			grid.baseY = gridBaseY;

			static int lastResolution = gridResolution;
			if (gridResolution != lastResolution) {
				grid.resolution = gridResolution;
				grid.rebuild();
				lastResolution = gridResolution;
			}
			grid.update(objects);

			gridShader.Activate();
			glUniformMatrix4fv(gridCamLoc, 1, GL_FALSE, glm::value_ptr(camMat));
			grid.draw(gridCamLoc, gridColorLoc, gridBaseYLoc);
		}

		// --- draw orbit trails ---
		gridShader.Activate();
		glUniformMatrix4fv(gridCamLoc, 1, GL_FALSE, glm::value_ptr(camMat));
		for (auto& trail : trails)
			trail.draw(gridCamLoc, gridColorLoc);

		// ---------- ImGui controls ----------
		ImGui::Begin("Simulation Controls");
		ImGui::Text("FPS: %.1f", 1.0f / dtReal);
		ImGui::Text("Bodies: %d", (int)objects.size());
		ImGui::Separator();

		// Scenario selector
		if (ImGui::BeginCombo("Scenario", scenarioName(static_cast<ScenarioType>(currentScenario)))) {
			for (int i = 0; i < static_cast<int>(ScenarioType::Count); i++) {
				bool selected = (i == currentScenario);
				if (ImGui::Selectable(scenarioName(static_cast<ScenarioType>(i)), selected))
					currentScenario = i;
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::Separator();

		ImGui::SliderFloat("Time Scale (yr/s)", &timeScale, 0.0f, 5.0f);
		ImGui::SliderFloat("Visual Body Scale", &visualScale, 0.01f, 5.0f);
		ImGui::SliderFloat("Curvature", &curvatureFactor, 0.0f, 3.0f);
		ImGui::SliderInt("Trail Length", &trailLength, 0, 2000);
		ImGui::Checkbox("Show Spacetime Grid", &showGrid);
		ImGui::SliderFloat("Grid Size", &gridSize, 1.0f, 100.0f);
		ImGui::SliderFloat("Grid Base Y", &gridBaseY, -5.0f, 5.0f);
		ImGui::SliderInt("Grid Resolution", &gridResolution, 10, 200);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ---------- cleanup ----------
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glowVAO.Delete();
	glowVBO.Delete();
	glowEBO.Delete();

	objects.clear();
	bodyShader.Delete();
	gridShader.Delete();
	glowShader.Delete();
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

	GLFWwindow* window = glfwCreateWindow((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, "Star System Simulation", nullptr, nullptr);
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