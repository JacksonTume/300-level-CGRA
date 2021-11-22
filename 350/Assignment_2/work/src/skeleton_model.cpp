#define GLM_ENABLE_EXPERIMENTAL 

// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp> 

// project
#include "cgra/cgra_geometry.hpp"
#include "skeleton_model.hpp"

#include <iostream>

using namespace std;
using namespace glm;
using namespace cgra;


void skeleton_model::draw(const mat4 &view, const mat4 &proj) {
	// set up the shader for every draw call
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

	// if the skeleton is not empty, then draw
	if (!skel.bones.empty()) {
		drawBone(view, 0);
	}
}


void skeleton_model::drawBone(const mat4 &parentTransform, int boneid) {
	// TODO:
	skeleton_bone currentBone = skel.bones.at(boneid);

	// the model matrix, holds the parents transform initially
	mat4 modelMatrix(parentTransform);
	// set the transformation (location) of the joint
	// Multiply by 25 to "space out" the joints
	modelMatrix = translate(modelMatrix, (currentBone.direction * currentBone.length) * vec3(25.f));
	modelMatrix = scale(modelMatrix, vec3(.5f)); // scale down a little bit so it's not huge

	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelMatrix));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(normalize(vec3(0, 255,255)))); // set cyan colour

	// scale the modelMatrix back up to the original size so it doesn't affect the next bone
	modelMatrix = scale(modelMatrix, vec3(2.f));

	// if its a leaf node (no children), then it doesn't have anything to join with. so we don't need to draw a joint
	if (!currentBone.children.empty())
	{
		// draw joint
		drawSphere();

		// draw the axes
		// z axis
		mat4 arrowCylinderZ(modelMatrix);

		arrowCylinderZ = rotate(arrowCylinderZ, degrees(skel.bones.at(boneid + 1).basis.z), vec3(0, 0, 1));
		arrowCylinderZ = rotate(arrowCylinderZ, degrees(skel.bones.at(boneid + 1).basis.y), vec3(0, 1, 0));
		arrowCylinderZ = rotate(arrowCylinderZ, degrees(skel.bones.at(boneid + 1).basis.x), vec3(1, 0, 0));

		arrowCylinderZ = scale(arrowCylinderZ, vec3(.2f, .2f, 1.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowCylinderZ));
		glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(normalize(vec3(0, 0, 255)))); // set blue colour

		drawCylinder();
		mat4 arrowConeZ(modelMatrix);

		arrowConeZ = rotate(arrowConeZ, degrees(skel.bones.at(boneid + 1).basis.z), vec3(0, 0, 1));
		arrowConeZ = rotate(arrowConeZ, degrees(skel.bones.at(boneid + 1).basis.y), vec3(0, 1, 0));
		arrowConeZ = rotate(arrowConeZ, degrees(skel.bones.at(boneid + 1).basis.x), vec3(1, 0, 0));


		arrowConeZ = translate(arrowConeZ, vec3(0.f, 0.f, 1.5f));
		arrowConeZ = scale(arrowConeZ, vec3(.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowConeZ));

		drawCone();

		// x axis
		mat4 arrowCylinderX(modelMatrix);

		arrowCylinderX = rotate(arrowCylinderX, degrees(skel.bones.at(boneid + 1).basis.z), vec3(0, 0, 1));
		arrowCylinderX = rotate(arrowCylinderX, degrees(skel.bones.at(boneid + 1).basis.y), vec3(0, 1, 0));
		arrowCylinderX = rotate(arrowCylinderX, degrees(skel.bones.at(boneid + 1).basis.x), vec3(1, 0, 0));

		arrowCylinderX = rotate(arrowCylinderX, radians(90.f), vec3(0, 1, 0));
		arrowCylinderX = scale(arrowCylinderX, vec3(.2f, .2f, 1.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowCylinderX));
		glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(normalize(vec3(255, 0, 0)))); // set red colour

		drawCylinder();
		mat4 arrowConeX(modelMatrix);

		arrowConeX = rotate(arrowConeX, degrees(skel.bones.at(boneid + 1).basis.z), vec3(0, 0, 1));
		arrowConeX = rotate(arrowConeX, degrees(skel.bones.at(boneid + 1).basis.y), vec3(0, 1, 0));
		arrowConeX = rotate(arrowConeX, degrees(skel.bones.at(boneid + 1).basis.x), vec3(1, 0, 0));

		arrowConeX = rotate(arrowConeX, radians(90.f), vec3(0, 1, 0));
		arrowConeX = translate(arrowConeX, vec3(0.f, 0.f, 1.5f));
		arrowConeX = scale(arrowConeX, vec3(.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowConeX));

		drawCone();

		// y axis
		mat4 arrowCylinderY(modelMatrix);

		arrowCylinderY = rotate(arrowCylinderY, degrees(skel.bones.at(boneid + 1).basis.z), vec3(0, 0, 1));
		arrowCylinderY = rotate(arrowCylinderY, degrees(skel.bones.at(boneid + 1).basis.y), vec3(0, 1, 0));
		arrowCylinderY = rotate(arrowCylinderY, degrees(skel.bones.at(boneid + 1).basis.x), vec3(1, 0, 0));

		arrowCylinderY = rotate(arrowCylinderY, radians(-90.f), vec3(1, 0, 0));
		arrowCylinderY = scale(arrowCylinderY, vec3(.2f, .2f, 1.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowCylinderY));
		glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(normalize(vec3(0, 255, 0)))); // set green colour

		drawCylinder();
		mat4 arrowConeY(modelMatrix);

		arrowConeY = rotate(arrowConeY, degrees(skel.bones.at(boneid + 1).basis.z), vec3(0, 0, 1));
		arrowConeY = rotate(arrowConeY, degrees(skel.bones.at(boneid + 1).basis.y), vec3(0, 1, 0));
		arrowConeY = rotate(arrowConeY, degrees(skel.bones.at(boneid + 1).basis.x), vec3(1, 0, 0));

		arrowConeY = rotate(arrowConeY, radians(-90.f), vec3(1, 0, 0));
		arrowConeY = translate(arrowConeY, vec3(0.f, 0.f, 1.5f));

		arrowConeY = scale(arrowConeY, vec3(.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(arrowConeY));

		drawCone();
	}

	// now we can draw the bones
	mat4 bone(modelMatrix);

	bone *= orientation(-currentBone.direction, vec3(0, 0, 1)); // rotate the bone to face the desired direction
	bone = scale(bone, vec3(.33f)); // scale down a bit so it's not so fat

	// need to multiply length by 25 as thats what we did to "space out" the joints,
	// then we need to multiply by 3 as we scaled the bones down by 3 to make them thinner
	bone = scale(bone, vec3(1, 1, currentBone.length * 25.f * 3.f));

	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(bone));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(normalize(vec3(0.7f)))); // set grey colour

	drawCylinder();

	if (!currentBone.children.empty())
	{
		for (int i = 0; i < currentBone.children.size() ; i++)
		{
			drawBone(modelMatrix, currentBone.children.at(i));
		}
	}

}