#pragma once

#include <glm/glm.hpp>

class Camera {
public:
	Camera();

	void update();

	glm::mat4 getProjView() const;

	void renderImGui();

private:
	glm::vec3 mPosition;
	glm::vec3 mFront;
	glm::vec3 mUp;
	glm::vec3 mRight;
	float mYaw, mPitch;

	float mMouseSensitivity;
	float mSpeed;
	float mZoom;

	void updateCameraVectors();
};