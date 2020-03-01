#pragma once

class Camera
{
public:
	Camera();

	void SetPosition(XMFLOAT3 position);
	void SetRotation(XMFLOAT3 euler);
	void SetTransform(Transform* transform);
	void SetFrustum(float fov, float nearz = 0.1f, float farz = 1000.0f);
	
	XMMATRIX GetProjectionMatrix(float aspectRatio);
	XMMATRIX LookAt(XMFLOAT3 position);
	XMMATRIX GetViewMatrix();
	
private:
	Transform m_transform;
	XMFLOAT3 m_up;
	XMFLOAT3 m_eyeDirection;

	float m_fov;
	float m_near;
	float m_far;

	bool m_dirty;
};