#define GLM_ENABLE_EXPERIMENTAL

// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4& view, const glm::mat4 proj, glm::vec3 position)
{
	mat4 modelview = view * modelTransform;

	modelview = scale(modelview, vec3(0.1));
	modelview = translate(modelview, position);

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	mesh.draw(); // draw
}

Application::Application(GLFWwindow* window) : m_window(window)
{
	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();
	m_shader = shader;
	m_first_model.shader = shader;
	m_first_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_first_model.color = vec3(1, 0, 0);

	m_second_model = m_first_model;
	m_second_model.color = vec3(0, 0, 1);

	m_control_points.emplace_back(vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)); // start point

	m_control_points.emplace_back(vec3(2.f, .5f, 3.f), vec3(1.f, 1.f, 1.f));
	m_control_points.emplace_back(vec3(1.f, 1.5f, -1.f), vec3(1.f, 1.f, 1.f));
	m_control_points.emplace_back(vec3(4.5f, 3.f, -2.f), vec3(1.f, 1.f, 1.f));

	m_control_points.emplace_back(vec3(7.f, -2.f, 5.f), vec3(1.f, 0.f, 0.f)); // end point

	for (float t = 0.0f; t < m_control_points.size() - 1; t += 0.01)
	{
		m_spline_points.push_back(getSplinePoint(t));
		m_value.push_back(t);
	}

	float distance = 0.f;

	for (int i = 0; i < m_spline_points.size() - 1; i++)
	{
		 distance += length(m_spline_points.at(i) - m_spline_points.at(i + 1));

		 m_speed_length.push_back(distance);
	}

	for (float t = 0.0f; t < m_control_points.size() - 1.01; t += 0.01)
	{
		int i0 = (int) (t / 0.01);
		int i1 = (int) (t / 0.01 + 0.5);

		float s =  (m_speed_length[i1] - m_speed_length[i0]);
		float ts = s * (i1 - i0) + i0;
		ts /= 100.f;

		m_speed_points.push_back(getSplinePoint(ts));
	}

	m_spline.shader = shader;
	int count = 0;
	mesh_builder mb;

	for (float i = 0; i < m_spline_points.size() - 0.5; i += 0.5)
	{
		mb.push_index(i);

		if (i == 0) i = 0.5;
	}

	for (vec3 point : m_spline_points)
	{
		mesh_vertex v;

		v.pos = point * 10.f;
		mb.push_vertex(v);

		count++;
	}

	m_spline.mesh = mb.build();
	m_spline.mesh.mode = GL_LINES;
}

void Application::draw(glm::vec3 position, glm::vec3 scale, const glm::mat4& view, const glm::mat4& proj, glm::vec3 color)
{
	mat4 modelMatrix(view);

	glUseProgram(m_shader);

	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::scale(modelMatrix, scale);

	glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "uModelViewMatrix"), 1, false, value_ptr(modelMatrix));
	glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1, value_ptr((color)));

	drawSphere();
}

glm::vec3 Application::getSplinePoint(float t)
{
	int i0 = clamp<int>(t - 1, 0, m_control_points.size() - 1);
	int i1 = clamp<int>(t, 0, m_control_points.size() - 1);
	int i2 = clamp<int>(t + 1, 0, m_control_points.size() - 1);
	int i3 = clamp<int>(t + 2, 0, m_control_points.size() - 1);

	float local_t = fract(t);

	return glm::catmullRom(
		m_control_points[i0].position,
		m_control_points[i1].position,
		m_control_points[i2].position,
		m_control_points[i3].position,
		local_t
	);
}

void Application::render()
{

	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = lookAt(vec3(-5.f ,0, m_distance), m_current_model_pos / 10.f, vec3(0, 1, 0));

	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw control points
	for (sPoint point : m_control_points)
	{
		draw(point.position, vec3(.2f), view, proj, point.color);
	}

	// current point of model
	if (m_model_index >= m_speed_points.size() - 1) m_model_index = 0;

	m_current_model_pos = m_spline_points.at(m_model_index) * 10.f;
	m_current_model_pos_second_model = m_speed_points.at(m_model_index) * 10.f;

	// draw model at current point
	m_first_model.draw(view, proj, m_current_model_pos);

	if(m_show_comp)
		m_second_model.draw(view, proj, m_current_model_pos_second_model);

	m_spline.draw(view, proj, vec3(0)
	);

	m_model_index++;
}

void Application::renderGUI()
{

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	ImGui::Checkbox("Show Comp", &m_show_comp);

	// finish creating window
	ImGui::End();
}

void Application::cursorPosCallback(double xpos, double ypos)
{
	if (m_leftMouseDown)
	{
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods)
{
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset)
{
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods)
{
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c)
{
	(void)c; // currently un-used
}


