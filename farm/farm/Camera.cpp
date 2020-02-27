#include "stdafx.h"
#include "Camera.h"


Camera::Camera() :
	m_fov(0.8f),
	m_near(0.1f),
	m_far(1000.0f),
	m_dirty(false),
	m_up(0, 1, 0),
	m_eyeDirection(0, 0, 1),
	m_transform()
{
}

void Camera::SetFrustum(float fov, float nearz, float farz)
{
	m_fov = fov;
	m_near = nearz;
	m_far = farz;

	m_dirty = true;
}

void Camera::SetPosition(XMFLOAT3 position)
{
	m_transform.position = position;

	m_dirty = true;
}

void Camera::SetTransform(Transform* transform)
{
	m_transform = *transform;

	m_dirty = true;
}

XMMATRIX Camera::GetProjectionMatrix(float aspectRatio) 
{
	return XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_near, m_far);
}

XMMATRIX Camera::LookAt(XMFLOAT3 position)
{
	XMVECTOR pos = XMVectorSet(m_transform.position.x, m_transform.position.y, m_transform.position.z, 1.0f);

	return XMMatrixLookAtLH(
		pos,
		XMLoadFloat3(&position),
		XMLoadFloat3(&m_up));
}

