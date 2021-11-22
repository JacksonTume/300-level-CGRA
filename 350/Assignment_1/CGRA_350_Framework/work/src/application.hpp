
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "skeleton_model.hpp"
#include "stb_image.h"

#include <iostream>
#include <string>

#define PI glm::pi<float>()
#define MAX_DIVISIONS 250
#define MIN_DIVISIONS 25
#define RADIUS 1.0f


// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{0.7};
	glm::mat4 modelTransform{1.0};
	GLuint texture;

	void draw(const glm::mat4 &view, const glm::mat4 proj);
};

// Main application class
//
class Application {
private:

	struct wavefront_vertex
	{
		unsigned int p, n, t;
	};


	// window
	glm::vec2 m_windowsize;
	GLFWwindow *m_window;

	// oribital camera
	float m_pitch = .86;
	float m_yaw = -.86;
	float m_distance = 5;

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	// geometry
	basic_model m_model;

	// my code
	bool m_update_mesh = false;
	bool m_sphere_lat_long = true;
	bool m_sphere_from_cube = false;

	int m_num_lat = MAX_DIVISIONS / 2;
	int m_num_long = MAX_DIVISIONS / 2;
	int m_num_subdivision = 100;


public:
	// setup
	Application(GLFWwindow *);

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

	cgra::mesh_builder sphereLatLong();

	cgra::mesh_builder sphereFromCube();

	void makeCubeFace(std::vector<glm::vec3> *positions, std::vector<glm::vec2> *uvs, std::vector<glm::vec3> *faces, float step, int side);

	glm::vec3 getPoint(int i, int j, float theta, float phi);

	glm::vec2 getUVs(glm::vec3 vertex);

	GLuint loadPNG(const char* filename);

	bool m_default_shader = true;
	bool m_cook_torrance = false;
	bool m_oren_nayar = false;
	bool m_bTexture = false;
};