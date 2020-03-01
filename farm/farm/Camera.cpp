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

void Camera::SetRotation(XMFLOAT3 euler)
{
	m_transform.rotation = XMFLOAT3{
		euler.x / 180.0f * XM_PI,
		euler.y / 180.0f * XM_PI,
		euler.z / 180.0f * XM_PI
	};
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
	return XMMatrixLookAtLH(
		XMLoadFloat3(&m_transform.position),
		XMLoadFloat3(&position),
		XMLoadFloat3(&m_up));
}

// Transform 값을 이용해 view matrix를 얻는다
XMMATRIX Camera::GetViewMatrix()
{
	XMVECTOR eye = XMLoadFloat3(&m_transform.position);
	XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_transform.rotation));
	
	forward = XMVector3TransformNormal(forward, rot);
	up = XMVector3TransformNormal(up, rot);

	return XMMatrixLookAtLH(
		eye,
		eye + forward,
		up);
}
