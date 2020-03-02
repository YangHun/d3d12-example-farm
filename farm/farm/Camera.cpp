#include "stdafx.h"
#include "Camera.h"


#include <iostream>

Camera::Camera() :
	m_fov(0.8f),
	m_near(0.1f),
	m_far(1000.0f),
	m_dirty(false),
	m_up(0, 1, 0),
	m_forward(0, 0, 1),
	m_transform()
{
	m_nextMousePos.x = 0;
	m_nextMousePos.y = 0;
}

void Camera::SetFrustum(float fov, float nearz, float farz)
{
	m_fov = fov;
	m_near = nearz;
	m_far = farz;
}

void Camera::SetPosition(XMFLOAT3 position)
{
	m_transform.position = position;
	m_dirty = true;
	//CalculateCameraAxis();
}

void Camera::SetRotation(XMFLOAT3 euler)
{
	m_transform.rotation = XMFLOAT3{
		euler.x / 180.0f * XM_PI,
		euler.y / 180.0f * XM_PI,
		euler.z / 180.0f * XM_PI
	};
	m_dirty = true;
	//CalculateCameraAxis();
}

void Camera::SetTransform(Transform* transform)
{
	m_transform = *transform;
	m_dirty = true;
	//CalculateCameraAxis();
}

XMMATRIX Camera::GetProjectionMatrix(float aspectRatio) 
{
	return XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_near, m_far);
}

XMMATRIX Camera::LookAt(XMFLOAT3 position)
{
	// re-calculate up, forward, right vector to look at specific position.

	XMVECTOR w = XMVector3Normalize(XMLoadFloat3(&position) - XMLoadFloat3(&m_transform.position));
	XMVECTOR u = XMVector3Cross(XMLoadFloat3(&m_up), w);
	XMVECTOR v = XMVector3Cross(w, u);

	XMStoreFloat3(&m_u, u);
	XMStoreFloat3(&m_v, v);
	XMStoreFloat3(&m_w, w);


	return XMMatrixLookAtLH(
		XMLoadFloat3(&m_transform.position),
		XMLoadFloat3(&position),
		XMLoadFloat3(&m_up));
}

// Transform 값을 이용해 view matrix를 얻는다
XMMATRIX Camera::GetViewMatrix()
{
	XMVECTOR eye = XMLoadFloat3(&m_transform.position);
	XMVECTOR forward = XMLoadFloat3(&m_w);
	XMVECTOR up = XMLoadFloat3(&m_up);

	return XMMatrixLookAtLH(
		eye,
		eye + forward,
		up);
}

void Camera::Update()
{
	XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR forward = XMLoadFloat3(&m_w);
	XMVECTOR right = XMLoadFloat3(&m_u);
	
	if (m_pressedW) dir += forward;
	if (m_pressedA) dir -= right;
	if (m_pressedS) dir -= forward;
	if (m_pressedD) dir += right;

	dir = XMVector3Normalize(dir) * 0.1f;
		
	if (!XMVector3Equal(dir, XMVectorZero())) {
		XMFLOAT3 pos;
		XMStoreFloat3(&pos, XMLoadFloat3(&m_transform.position) + dir);

		SetPosition(pos);
	}



	if (m_dirty)
	{
		CalculateCameraAxis();
		m_dirty = false;
	}
}


void Camera::CalculateCameraAxis()
{
	// calculate up, forward, right vector using current transform.
	XMVECTOR eye = XMLoadFloat3(&m_transform.position);
	XMVECTOR w = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR j = XMLoadFloat3(&m_up);

	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_transform.rotation));

	w = XMVector3TransformNormal(w, rot);

	XMVECTOR u = XMVector3Normalize(XMVector3Cross(j, w));
	XMVECTOR v = XMVector3Cross(w, u);

	XMStoreFloat3(&m_u, u);
	XMStoreFloat3(&m_v, v);
	XMStoreFloat3(&m_w, w);
}

void Camera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_pressedW = true;
		break;
	case 'A':
		m_pressedA = true;
		break;
	case 'S':
		m_pressedS = true;
		break;
	case 'D':
		m_pressedD = true;
		break;
	}
}

void Camera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_pressedW = false;
		break;
	case 'A':
		m_pressedA = false;
			break;
	case 'S':
		m_pressedS = false;
			break;
	case 'D':
		m_pressedD = false;
			break;
	}
}

void Camera::OnMouseMove(WPARAM state, int x, int y)
{	
	m_lastMousePos = m_nextMousePos;

	m_nextMousePos.x = x;
	m_nextMousePos.y = y;

	if ((state & MK_LBUTTON) != 0) {
		float dx = XMConvertToRadians(0.25f * static_cast<float>(m_nextMousePos.x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(m_nextMousePos.y - m_lastMousePos.y));

		m_transform.rotation.y = m_transform.rotation.y + dx;
		m_transform.rotation.x = m_transform.rotation.x + dy;

		//m_transform.rotation.y = Clamp(m_transform.rotation.y + dx, 90.0f, -90.0f);
		//m_transform.rotation.x = Clamp(m_transform.rotation.x + dy, 90.0f, -90.0f);

		m_dirty = true;
	}


}
