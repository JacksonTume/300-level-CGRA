
#pragma once
#define GLM_ENABLE_EXPERIMENTAL

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/spline.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "skeleton_model.hpp"

// std
#include <vector>


// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model
{
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	void draw(const glm::mat4& view, const glm::mat4 proj, glm::vec3 position);
};

struct sPoint
{
	glm::vec3 position;
	glm::vec3 color;

	sPoint(glm::vec3 pos, glm::vec3 col) : position(pos), color(col) {}
};

// Main application class
//
class Application
{
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow* m_window;

	// oribital camera
	float m_pitch = .86;
	float m_yaw = -.86;
	float m_distance = 10;

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	// geometry
	basic_model m_first_model;
	basic_model m_second_model;

	basic_model m_spline;

	std::vector<sPoint> m_control_points;

	std::vector<glm::vec3> m_spline_points;
	std::vector<glm::vec3> m_speed_points;

	std::vector<float> m_speed_length;
	std::vector<float> m_value;

	bool m_show_comp = false;

	GLuint m_shader = 0;

	int m_model_index = 0;
	
	glm::vec3 m_current_model_pos;
	glm::vec3 m_current_model_pos_second_model;

	
public:
	// setup
	Application(GLFWwindow*);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void render();
	void renderGUI();

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);

	void draw(glm::vec3 position, glm::vec3 scale, const glm::mat4& view, const glm::mat4& proj, glm::vec3 color);

	glm::vec3 getSplinePoint(float t);
};