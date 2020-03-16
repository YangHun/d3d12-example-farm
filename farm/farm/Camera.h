#pragma once

#include "Component.h"

class Camera : public Observer
{
public:
	Camera(CD3DX12_VIEWPORT* viewport);

	void OnNotify(Object* object, E_Event event) override;

	void SetPosition(XMFLOAT3 position);
	void SetRotation(XMFLOAT3 euler);
	void SetTransform(Transform* transform);
	void SetFrustum(float fov, float nearz = 0.1f, float farz = 1000.0f);
	
	XMMATRIX GetProjectionMatrix();
	XMMATRIX LookAt(XMFLOAT3 position);
	XMMATRIX GetViewMatrix();
	XMFLOAT3 GetEyePosition();

	XMFLOAT3 GetRightVector()
	{
		return m_u;
	}
	
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
	void OnMouseMove(WPARAM state, int x, int y);
	void Update(float dt);

	void ResetMousePos(POINT point);

	XMFLOAT2 ScreenToViewport(int x, int y);

	float GetWidth() { return m_pViewport == nullptr ? 0 : m_pViewport->Width; }
	float GetHeight() { return m_pViewport == nullptr ? 0 : m_pViewport->Height; }

	std::wstring PrintInformation();

private:
	void CalculateCameraAxis();

private:
	Transform m_transform;
	XMFLOAT3 m_up;
	XMFLOAT3 m_forward;

	XMFLOAT3 m_u;	// right
	XMFLOAT3 m_v;	// up (eye space axis)
	XMFLOAT3 m_w;	// forward

	float m_fov;
	float m_near;
	float m_far;

	bool m_dirty;

	bool m_pressedW = false;
	bool m_pressedA = false;
	bool m_pressedS = false;
	bool m_pressedD = false;
	
	POINT m_initialMousePos;
	POINT m_prevMousePos;
	POINT m_nextMousePos;

	// used to calculate screen-to-world coordinate translation
	CD3DX12_VIEWPORT* m_pViewport;

	bool m_lockRotation = false;

public:
	GameObject* picked = nullptr;
};


// helper function
inline float Clamp(float value, float floor, float ceil)
{
	if (value > ceil) return ceil;
	if (value < floor) return floor;
	return value;
}

inline int Clamp(int value, int floor, int ceil)
{
	if (value > ceil) return ceil;
	if (value < floor) return floor;
	return value;
}