
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

#include "skeleton.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4& view, const glm::mat4 proj)
{
	mat4 modelview = view * modelTransform;

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

	m_model.shader = shader;
	m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(1, 0, 0);

	shader_builder myShaderB;
	myShaderB.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	myShaderB.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	boneShader = myShaderB.build();

	m_skeleton_T_Pose.shader = boneShader; // boneShader just has the default shader for now, but it gets changed around in the skeleton_model class
	m_skeleton_T_Pose.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//priman.asf"));

	m_skeleton_Dabbing_Pose.shader = boneShader;
	m_skeleton_Dabbing_Pose.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//Completion_Poses//dabbing_pose.asf"));

	m_skeleton_Jumping_Pose.shader = boneShader;
	m_skeleton_Jumping_Pose.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//Completion_Poses//jumping_pose.asf"));

	m_skeleton_Punching_Pose.shader = boneShader;
	m_skeleton_Punching_Pose.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//Completion_Poses//punching_pose.asf"));

	m_skeleton_Walking_Pose.shader = boneShader;
	m_skeleton_Walking_Pose.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//Completion_Poses//walking_pose.asf"));

	m_skeleton_Seated_Pose.shader = boneShader;
	m_skeleton_Seated_Pose.skel = skeleton_data(CGRA_SRCDIR + std::string("/res//assets//Completion_Poses//seated_pose.asf"));
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
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw, vec3(0, 1, 0));

	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);


	glUseProgram(boneShader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(boneShader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(boneShader, "uModelViewMatrix"), 1, false, value_ptr(view));
	glUniform3fv(glGetUniformLocation(boneShader, "uColor"), 1, value_ptr(glm::vec3(0.7)));

	if (m_show_T)
	{
		m_skeleton_T_Pose.draw(view, proj);
	}
	else if (m_show_Dabbing)
	{
		m_skeleton_Dabbing_Pose.draw(view, proj);
	}
	else if (m_show_Jumping)
	{
		m_skeleton_Jumping_Pose.draw(view, proj);
	}
	else if (m_show_Punching)
	{
		m_skeleton_Punching_Pose.draw(view, proj);
	}
	else if (m_show_Walking)
	{
		m_skeleton_Walking_Pose.draw(view, proj);
	}
	else if (m_show_Seated)
	{
		m_skeleton_Seated_Pose.draw(view, proj);
	}
}


void Application::renderGUI()
{

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 210), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);
	ImGui::Separator();


	// change the onscreen characters pose
	if (ImGui::Checkbox("T-Pose", &m_show_T))
	{
		m_show_Dabbing = false;
		m_show_Jumping = false;
		m_show_Punching = false;
		m_show_Walking = false;
		m_show_Seated = false;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Dabbing", &m_show_Dabbing))
	{
		m_show_T = false;
		m_show_Jumping = false;
		m_show_Punching = false;
		m_show_Walking = false;
		m_show_Seated = false;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Jumping", &m_show_Jumping))
	{
		m_show_T = false;
		m_show_Dabbing = false;
		m_show_Punching = false;
		m_show_Walking = false;
		m_show_Seated = false;
	}
	if (ImGui::Checkbox("Punch ", &m_show_Punching))
	{
		m_show_T = false;
		m_show_Dabbing = false;
		m_show_Jumping = false;
		m_show_Walking = false;
		m_show_Seated = false;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Walking", &m_show_Walking))
	{
		m_show_T = false;
		m_show_Dabbing = false;
		m_show_Jumping = false;
		m_show_Punching = false;
		m_show_Seated = false;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Seated", &m_show_Seated))
	{
		m_show_T = false;
		m_show_Dabbing = false;
		m_show_Jumping = false;
		m_show_Punching = false;
		m_show_Walking = false;
	}

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
