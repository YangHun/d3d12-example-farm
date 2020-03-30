#include "stdafx.h"
#include "Camera.h"


#include <iostream>

Camera::Camera(CD3DX12_VIEWPORT* viewport) :
	m_fov(0.8f),
	m_near(0.1f),
	m_far(1000.0f),
	m_dirty(true),
	m_up(0, 1, 0),
	m_u(1, 0, 0),
	m_v(0, 1, 0),
	m_w(0, 0, 1),
	m_forward(0, 0, 1),
	m_transform(),
	m_pViewport(viewport)
{
	m_initialMousePos.x = 1000;
	m_initialMousePos.y = 800;
	m_prevMousePos = m_nextMousePos = m_initialMousePos;
}

void Camera::OnNotify(Object* object, E_Event event)
{	
	switch (event)
	{
	case E_Event::TABLE_INTERACT_CLICKED:
		m_lockRotation = !m_lockRotation;
		ShowCursor(m_lockRotation);
		break;
	}
}

void Camera::SetFrustum(float fov, float nearz, float farz)
{
	m_fov = fov;
	m_near = nearz;
	m_far = farz;

	BoundingFrustum::CreateFromMatrix(m_frustum, GetProjectionMatrix());
}

bool Camera::FrustumCullTest(GameObject* obj)
{
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(GetViewMatrix()), GetViewMatrix());
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(obj->GetWorldMatrix()), obj->GetWorldMatrix());
	XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

	BoundingFrustum localSpaceFrustum;
	m_frustum.Transform(localSpaceFrustum, viewToLocal);

	return (localSpaceFrustum.Contains(obj->GetCollider()->GetBoundBox()) != DirectX::DISJOINT);
}

void Camera::SetPosition(XMFLOAT3 position)
{
	m_transform.position = position;
	m_dirty = true;
}

void Camera::SetRotation(XMFLOAT3 euler)
{
	m_transform.rotation = XMFLOAT3{
		euler.x / 180.0f * XM_PI,
		euler.y / 180.0f * XM_PI,
		euler.z / 180.0f * XM_PI
	};
	m_dirty = true;
}

void Camera::SetTransform(const Transform& transform)
{
	m_transform = transform;
	m_dirty = true;
}

XMMATRIX Camera::GetProjectionMatrix() 
{
	float aspectRatio = m_pViewport->Width / m_pViewport->Height;
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
	//XMVECTOR up = XMLoadFloat3(&m_v);

	return XMMatrixLookAtLH(
		eye,
		eye + forward,
		up);
}

void Camera::Update(float dt)
{
	if (m_dirty)
	{
		if (!m_lockRotation) {
			float dx = XMConvertToRadians(0.25f * static_cast<float>(m_nextMousePos.x - m_prevMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(m_nextMousePos.y - m_prevMousePos.y));
			m_transform.rotation.y = m_transform.rotation.y + dx;
			m_transform.rotation.x = m_transform.rotation.x - dy;
		}
		CalculateCameraAxis();
		m_dirty = false;
	}

	
	XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR forward = XMLoadFloat3(&m_w);
	XMVECTOR right = XMLoadFloat3(&m_u);
	
	if (m_pressedW) dir += forward;
	if (m_pressedA) dir -= right;
	if (m_pressedS) dir -= forward;
	if (m_pressedD) dir += right;
			
	if (!XMVector3Equal(dir, XMVectorZero())) {
		dir = XMVector3Normalize(dir) * 0.2f;
		XMFLOAT3 pos;
		XMStoreFloat3(&pos, XMLoadFloat3(&m_transform.position) + dir);
		m_transform.position = pos;
	}

	// camera height is always fixed.
	m_transform.position.y = 4.0f;
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
	//m_dirty = true;
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
	m_prevMousePos = m_nextMousePos;

	m_nextMousePos.x = x;
	m_nextMousePos.y = y;

	if ((m_nextMousePos.x != m_prevMousePos.x) ||
		(m_nextMousePos.y != m_prevMousePos.y))
		m_dirty = true;
	
}


std::wstring Camera::PrintInformation()
{
	std::wostringstream oss;
	oss << "position: (" << m_transform.position.x << ", " << m_transform.position.y << ", " << m_transform.position.z << ")\n";
	oss << "rotation: (" << m_transform.rotation.x << ", " << m_transform.rotation.y << ", " << m_transform.rotation.z << ")\n";	
	oss << "picked: " << (picked == nullptr ? "(null)" :picked->GetName().c_str()) << "\n";

	return oss.str();
}

void Camera::ResetMousePos(POINT point)
{
	m_initialMousePos = point;
	m_prevMousePos = m_nextMousePos = m_initialMousePos;
}

XMFLOAT3 Camera::GetEyePosition()
{
	return m_transform.position;
}

XMFLOAT2 Camera::ScreenToViewport(int x, int y)
{
	return XMFLOAT2(
		((float)x * 2 / m_pViewport->Width) - 1.0f,
		-(((float)y * 2 / m_pViewport->Height) - 1.0f)	// 좌측 상단이 (0, 0) 이므로 inverse y
	);
}