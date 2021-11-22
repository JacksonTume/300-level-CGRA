
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

using namespace std;
using namespace cgra;
using namespace glm;

bool default_shader = true;
bool cook_torrence = false;
bool bTexture = false;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

GLuint TEXTURE, NORMAL;

void basic_model::draw(const glm::mat4& view, const glm::mat4 proj)
{
	mat4 modelview = view * modelTransform;
	glUseProgram(shader); // load shader and variables

	if (default_shader)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
		glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
	}
	else if (cook_torrence)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
		glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, value_ptr(lightPos));
	}
	else if (bTexture)
	{
		// no clue why its black??
		// but i've spent about 4 hours trying to fix it and it's broken :(
		glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
		glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
		glUniform1i(glGetUniformLocation(shader, "Texture"), 0);
		glUniform1i(glGetUniformLocation(shader, "NormalMap"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TEXTURE);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NORMAL);

	}
	mesh.draw(); // draw
}


Application::Application(GLFWwindow* window) : m_window(window)
{

	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = sphereLatLong().build();
	m_model.color = vec3(1, 0, 0);
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

	// the mesh has changed and needs updating
	if (m_update_mesh)
	{
		// the sphere_lat_long mesh needs updating
		if (m_sphere_lat_long)
		{
			m_model.mesh = sphereLatLong().build();
			m_update_mesh = false;
		}
		else if (m_sphere_from_cube)
		{
			m_update_mesh = false;
			m_model.mesh = sphereFromCube().build();
		}

		// no mesh has been selected, so just return and don't draw anything
		else { return; }
	}
	// keep the colour red
	m_model.color = vec3(1, 0, 0);

	// draw the model
	m_model.draw(view, proj);
}


void Application::renderGUI()
{

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiSetCond_Once);
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

	// my code
	if (ImGui::Checkbox("Sphere Lat Long", &m_sphere_lat_long))
	{
		m_sphere_from_cube = false;
		m_update_mesh = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Sphere From Cube", &m_sphere_from_cube))
	{
		m_sphere_lat_long = false;
		m_update_mesh = true;
	}

	if (ImGui::Checkbox("Default", &m_default_shader))
	{
		m_cook_torrance = false;
		cook_torrence = false;
		m_oren_nayar = false;
		bTexture = false;
		m_bTexture = false;
		default_shader = true;

		shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
		GLuint shader = sb.build();
		m_model.shader = shader;

		m_update_mesh = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Cook-Torrance", &m_cook_torrance))
	{
		m_default_shader = false;
		default_shader = false;
		m_oren_nayar = false;
		bTexture = false;
		m_bTexture = false;
		cook_torrence = true;

		shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//cook-torrance.glsl"));
		GLuint shader = sb.build();

		m_model.shader = shader;
		m_model.color = vec3(0, 1, 0);

		m_update_mesh = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Texture", &m_bTexture))
	{
		m_default_shader = false;
		default_shader = false;
		m_cook_torrance = false;
		cook_torrence = false;
		bTexture = true;

		shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//texture_frag.glsl"));
		GLuint shader = sb.build();
		m_model.shader = shader;

		//Texture tex(CGRA_SRCDIR + string("//res//textures//Texture.png"));
		//Texture norm(CGRA_SRCDIR + string("//res//textures//NormalMap.jpg"));

		TEXTURE = loadPNG(CGRA_SRCDIR "//res//textures//Texture.png");
		NORMAL = loadPNG(CGRA_SRCDIR "//res//textures//NormalMap.png");

		m_update_mesh = true;
	}

	if (ImGui::SliderInt("Divisions Lat", &m_num_lat, MIN_DIVISIONS, MAX_DIVISIONS))
		m_update_mesh = true;
	if (ImGui::SliderInt("Divisions Long", &m_num_long, MIN_DIVISIONS, MAX_DIVISIONS))
		m_update_mesh = true;

	if (ImGui::SliderInt("Number of Subdivisions", &m_num_subdivision, 25, 250))
		m_update_mesh = true;

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

	// 262 right
	if (key == 262 && action == 1)
	{
		m_num_subdivision += 5;
		m_update_mesh = true;
		cout << key << " ," << action << ", " << m_num_subdivision << endl;
	}
	// 263 left
	if (key == 263 && action == 1)
	{
		m_num_subdivision -= 5;
		m_update_mesh = true;
		cout << key << " ," << action << ", " << m_num_subdivision << endl;
	}
}


void Application::charCallback(unsigned int c)
{
	(void)c; // currently un-used
}

mesh_builder Application::sphereLatLong()
{
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> uvs;
	vector<wavefront_vertex> wv_vertices;

	vector<vec3> faces;

	float theta = (PI * PI) / m_num_lat;
	float phi = (2 * PI) / m_num_long;

	int index = 0;
	for (int i = 0; i < m_num_lat; i++)
	{
		for (int j = 0; j < m_num_long; j++)
		{
			// calculate the top points
			vec3 pointTopLeft = getPoint(i, j, theta, phi);
			vec3 pointTopRight = getPoint(i + 1, j, theta, phi);
			// and calculate the bottom points
			vec3 pointBotLeft = getPoint(i, j + 1, theta, phi);
			vec3 pointBotRight = getPoint(i + 1, j + 1, theta, phi);

			// store the positions
			positions.push_back(pointTopLeft);
			positions.push_back(pointTopRight);
			positions.push_back(pointBotLeft);
			positions.push_back(pointBotRight);

			// store the uvs
			uvs.push_back(getUVs(pointTopLeft));
			uvs.push_back(getUVs(pointTopRight));
			uvs.push_back(getUVs(pointBotLeft));
			uvs.push_back(getUVs(pointBotRight));

			// now make the face
			faces.push_back(vec3(index, index + 1, index + 2));
			faces.push_back(vec3(index + 2, index + 1, index + 3));
			index++;
		}
	}

	for (int i = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			wavefront_vertex v;
			v.p = i + j;
			v.n = -1;
			v.t = i + j;
			wv_vertices.push_back(v);
		}
	}

	// now we can generate the normals
	if (normals.empty())
	{
		normals.resize(positions.size(), vec3(0));

		for (size_t i = 0; i < wv_vertices.size() / 3; i++)
		{
			wavefront_vertex& a = wv_vertices[i * 3];
			wavefront_vertex& b = wv_vertices[i * 3 + 1];
			wavefront_vertex& c = wv_vertices[i * 3 + 2];

			// set the normal index to be the same as position index
			a.n = a.p;
			b.n = b.p;
			c.n = c.p;

			// calculate the face normal
			vec3 ab = positions[b.p] - positions[a.p];
			vec3 ac = positions[c.p] - positions[a.p];
			vec3 face_norm = cross(ab, ac);

			// contribute the face norm to each vertex
			float l = length(face_norm);
			if (l > 0)
			{
				face_norm / l;
				normals[a.n] += face_norm;
				normals[b.n] += face_norm;
				normals[c.n] += face_norm;
			}
		}

		// normalize the normals
		for (size_t i = 0; i < normals.size(); i++)
		{
			normals[i] = normalize(normals[i]);
		}
	}

	mesh_builder mb;

	for (unsigned int i = 0; i < positions.size(); ++i)
	{
		mb.push_index(i);
		mb.push_vertex(mesh_vertex{
			positions[wv_vertices[i].p],
			normals[wv_vertices[i].n],
			uvs[wv_vertices[i].t]
			});
	}

	return mb;
}

mesh_builder Application::sphereFromCube()
{
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> uvs;
	vector<wavefront_vertex> wv_vertices;

	vector<vec3> faces;

	float step = 2.f / (m_num_subdivision + 1);

	makeCubeFace(&positions, &uvs, &faces, step, 1);
	makeCubeFace(&positions, &uvs, &faces, step, 2);
	makeCubeFace(&positions, &uvs, &faces, step, 3);
	makeCubeFace(&positions, &uvs, &faces, step, 4);
	makeCubeFace(&positions, &uvs, &faces, step, 5);
	makeCubeFace(&positions, &uvs, &faces, step, 6);

	for (int i = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			wavefront_vertex v;
			v.p = i + j;
			v.n = -1;
			v.t = i + j;
			wv_vertices.push_back(v);
		}
	}

	// now we can generate the normals
	if (normals.empty())
	{
		normals.resize(positions.size(), vec3(0));

		for (size_t i = 0; i < wv_vertices.size() / 3; i++)
		{
			wavefront_vertex& a = wv_vertices[i * 3];
			wavefront_vertex& b = wv_vertices[i * 3 + 1];
			wavefront_vertex& c = wv_vertices[i * 3 + 2];

			// set the normal index to be the same as position index
			a.n = a.p;
			b.n = b.p;
			c.n = c.p;

			// calculate the face normal
			vec3 ab = positions[b.p] - positions[a.p];
			vec3 ac = positions[c.p] - positions[a.p];
			vec3 face_norm = cross(ab, ac);

			// contribute the face norm to each vertex
			float l = length(face_norm);
			if (l > 0)
			{
				face_norm / l;
				normals[a.n] += face_norm;
				normals[b.n] += face_norm;
				normals[c.n] += face_norm;
			}
		}

		// normalize the normals
		for (size_t i = 0; i < normals.size(); i++)
		{
			normals[i] = normalize(normals[i]);
		}
	}

	mesh_builder mb;

	for (unsigned int i = 0; i < wv_vertices.size(); ++i)
	{
		mb.push_index(i);
		mb.push_vertex(mesh_vertex{
			positions[wv_vertices[i].p],
			normals[wv_vertices[i].n],
			uvs[wv_vertices[i].t]
			});
	}

	return mb;
}

void Application::makeCubeFace(std::vector<glm::vec3>* positions, std::vector<glm::vec2>* uvs, std::vector<glm::vec3>* faces, float step, int side)
{

	for (int i = 0; i < m_num_subdivision; i++)
	{
		for (int j = 0; j < m_num_subdivision; j++)
		{
			vec3 pointTopLeft, pointTopRight, pointBotLeft, pointBotRight;
			switch (side)
			{
			case 1:
				pointTopLeft = vec3(i * step - 1, (j + 1) * step - 1, RADIUS);
				pointTopRight = vec3((i + 1) * step - 1, (j + 1) * step - 1, RADIUS);
				pointBotLeft = vec3(i * step - 1, j * step - 1, RADIUS);
				pointBotRight = vec3((i + 1) * step - 1, j * step - 1, RADIUS);
				break;
			case 2:
				pointTopLeft = vec3(i * step - 1, RADIUS, (j + 1) * step - 1);
				pointTopRight = vec3((i + 1) * step - 1, RADIUS, (j + 1) * step - 1);
				pointBotLeft = vec3(i * step - 1, RADIUS, j * step - 1);
				pointBotRight = vec3((i + 1) * step - 1, RADIUS, j * step - 1);
				break;
			case 3:
				pointTopLeft = vec3(RADIUS, i * step - 1, (j + 1) * step - 1);
				pointTopRight = vec3(RADIUS, (i + 1) * step - 1, (j + 1) * step - 1);
				pointBotLeft = vec3(RADIUS, i * step - 1, j * step - 1);
				pointBotRight = vec3(RADIUS, (i + 1) * step - 1, j * step - 1);
				break;
			case 4:
				pointTopLeft = vec3(-RADIUS, i * step - 1, (j + 1) * step - 1);
				pointTopRight = vec3(-RADIUS, (i + 1) * step - 1, (j + 1) * step - 1);
				pointBotLeft = vec3(-RADIUS, i * step - 1, j * step - 1);
				pointBotRight = vec3(-RADIUS, (i + 1) * step - 1, j * step - 1);
				break;
			case 5:
				pointTopLeft = vec3(i * step - 1, -RADIUS, (j + 1) * step - 1);
				pointTopRight = vec3((i + 1) * step - 1, -RADIUS, (j + 1) * step - 1);
				pointBotLeft = vec3(i * step - 1, -RADIUS, j * step - 1);
				pointBotRight = vec3((i + 1) * step - 1, -RADIUS, j * step - 1);
				break;
			case 6:
				pointTopLeft = vec3((i * step - 1), (j + 1) * step - 1, -RADIUS);
				pointTopRight = vec3(((i + 1) * step - 1), (j + 1) * step - 1, -RADIUS);
				pointBotLeft = vec3((i * step - 1), j * step - 1, -RADIUS);
				pointBotRight = vec3(((i + 1) * step - 1), j * step - 1, -RADIUS);
				break;
			default:
				break;
			}

			//cout << "(i, j): " << i << ", " << j << " : (" << pointTopLeft.x << ", " << pointTopLeft.y << ", " << pointTopLeft.z << endl;
			//cout << "(i, j): " << i << ", " << j << " : (" << pointTopRight.x << ", " << pointTopRight.y << ", " << pointTopRight.z << endl;
			//cout << "(i, j): " << i << ", " << j << " : (" << pointBotLeft.x << ", " << pointBotLeft.y << ", " << pointBotLeft.z << endl;
			//cout << "(i, j): " << i << ", " << j << " : (" << pointBotRight.x << ", " << pointBotRight.y << ", " << pointBotRight.z << endl;
			//cout << "=================case 3====================\n";

			int index = positions->size();

			// store the positions
			positions->push_back(normalize(pointTopLeft));
			positions->push_back(normalize(pointTopRight));
			positions->push_back(normalize(pointBotLeft));
			positions->push_back(normalize(pointBotRight));

			// store the uvs
			uvs->push_back(getUVs(pointTopLeft));
			uvs->push_back(getUVs(pointTopRight));
			uvs->push_back(getUVs(pointBotLeft));
			uvs->push_back(getUVs(pointBotRight));

			faces->push_back(vec3(index, index + 1, index + 2));
			faces->push_back(vec3(index + 2, index + 1, index + 3));
			faces->push_back(vec3(index, index + 2, index + 1));
			faces->push_back(vec3(index + 2, index + 3, index + 1));
		}
	}
}

vec3 Application::getPoint(int i, int j, float theta, float phi)
{
	float thetaStep = i * theta;
	float phiStep = j * phi;

	float x = sin(thetaStep) * cos(phiStep);
	float y = sin(thetaStep) * sin(phiStep);
	float z = cos(thetaStep);

	return RADIUS * vec3(x, y, z);
}

/* Got the numbers for this method from the following sources:
https://stackoverflow.com/questions/19357290/convert-3d-point-on-sphere-to-uv-coordinate
https://en.wikipedia.org/wiki/UV_mapping
*/
vec2 Application::getUVs(vec3 vertex)
{
	vec3 norm = normalize(vertex);

	//float u = atan2(norm.x, norm.y) / (2 * PI + .5f);
	//float v = asin(norm.y) * PI + 0.5;

	float u = 0.5 + (atan2(vertex.x, vertex.y) / (2 * PI));
	float v = 0.5 - asin(vertex.y) / PI;

	return vec2(u, v);
}

GLuint Application::loadPNG(const char* filename)
{
	// declare the textureID
	GLuint textureID;

	// create the opengl textures
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);


	stbi_set_flip_vertically_on_load(1);
	int width, height, channels;
	// load all the texture data into the buffer
	unsigned char* buffer = stbi_load(filename, &width, &height, &channels, 4);

	// DEBUG
	//cout << "Width/Height/Channels: [" << width << "/" << height << "/" << channels << "]\n";

	if (buffer)
		cout << "Success reading: " << filename << endl;
	else
		cout << "damn, what happen with : " << filename << endl;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);


	return textureID;
}
