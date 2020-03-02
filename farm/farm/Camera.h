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
	
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
	void Update();

private:
	void CalculateCameraAxis();

private:
	Transform m_transform;
	XMFLOAT3 m_up;
	XMFLOAT3 m_forward;

	XMFLOAT3 m_u;	// right
	XMFLOAT3 m_v;	// up (local axis)
	XMFLOAT3 m_w;	// forward


	float m_fov;
	float m_near;
	float m_far;

	bool m_dirty;

	bool m_pressedW = false;
	bool m_pressedA = false;
	bool m_pressedS = false;
	bool m_pressedD = false;
};