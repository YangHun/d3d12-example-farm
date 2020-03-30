#include "stdafx.h"
#include "Physics.h"

GameObject* Physics::Raycast(Camera* camera, int screenX, int screenY, E_RenderLayer layer)
{
	// generate the picking ray vector.
	Ray ray = GeneratePickingRay(camera, screenX, screenY);
	//Ray ray = GeneratePickingRay(camera, 640, 360);	// �߾�

	// raycast using Axis Aligned Bounding Boxes(AABB).
	auto scene = DirectXGame::GetCurrentScene();
	float distance = 1000.0f;

	GameObject* hit = nullptr;
	float min_distance = FLT_MAX;


	for (auto obj : scene->GetObjectsByLayer(layer))
	{
		if (!obj->IsActive()) continue;
		
		if (obj->GetCollider()->IsZero()) continue; // not have bounding box.
		
		// bounding box�� ���� object�� world transform�� ���� �� �����Ƿ�
		// ray�� object�� (W)^-1 �� ���� ���� �浹 ���θ� üũ�Ѵ�.
		
		Transform t = obj->GetTransform();
		XMMATRIX ST = obj->GetWorldMatrix();

		XMMATRIX InvST = XMMatrixInverse(&XMMatrixDeterminant(ST), ST);


		Ray _ray = ray;
		
		if (!XMMatrixIsIdentity(InvST)) {
			
			XMVECTOR _dir = XMVector4Transform(XMLoadFloat3(&_ray.direction), InvST);
			XMVECTOR _pos = XMVector3TransformCoord(XMLoadFloat3(&_ray.position), InvST);

			XMStoreFloat3(&_ray.direction, _dir);
			XMStoreFloat3(&_ray.position, _pos);
		}

		float distance = obj->GetCollider()->Pick(_ray);
		if (distance < min_distance)
		{
			min_distance = distance;
			hit = obj;
		}
	}

	return hit;
}	

float Physics::PickAABB(Ray ray, Collider::Bound bound)
{

	XMFLOAT3 _dir = ray.direction;
	XMFLOAT3 _pos = ray.position;

	// base case : P�� bounding box ���ο� ��ġ�� ���, true
	if (IsExist(_pos.x, bound.min.x, bound.max.x) 
		&& IsExist(_pos.y, bound.min.y, bound.max.y)
		&& IsExist(_pos.z, bound.min.z, bound.max.z))
		return true;

	// AABB üũ
	// ���� ���� ���� _pos + k * _dir (k is constant) �� ��Ÿ�� �� �ִ�.
	// x, y, z���� �ּ�/�ִ� �� bound�� ������ ������ 6���� ���� ���� k ���� ���� ���Ѵ�.
	float v[6];
	v[0] = (_dir.x != 0 ? ((bound.min.x - _pos.x) / _dir.x) : FLT_MAX);
	v[1] = (_dir.x != 0 ? ((bound.max.x - _pos.x) / _dir.x) : FLT_MAX);
	v[2] = (_dir.y != 0 ? ((bound.min.y - _pos.y) / _dir.y) : FLT_MAX);
	v[3] = (_dir.y != 0 ? ((bound.max.y - _pos.y) / _dir.y) : FLT_MAX);
	v[4] = (_dir.z != 0 ? ((bound.min.z - _pos.z) / _dir.z) : FLT_MAX);
	v[5] = (_dir.z != 0 ? ((bound.max.z - _pos.z) / _dir.z) : FLT_MAX);

	
	// near	: �ະ�� P�� ����� �� 3��(x_near, y_near, z_near)�� k�� �� �ִ밪
	// far	: �ະ�� P�� �ָ� �ִ� �� 3��(x_far, y_far, z_far)�� k�� �� �ּҰ�
	float _near, _far;
	_near = Max((v[0] < FLT_MAX ? 1.0f : -1.0f) * min(v[0], v[1]), 
		(v[2] < FLT_MAX ? 1.0f : -1.0f) * min(v[2], v[3]),
		(v[4] < FLT_MAX ? 1.0f : -1.0f) * min(v[4], v[5]));
	_far = Min( max(v[0], v[1]), max(v[2], v[3]), max(v[4], v[5]));

	return ((_far >= 0) && (_near <= _far))? _near : FLT_MAX;
}

Ray Physics::GeneratePickingRay(Camera* camera, int screenX, int screenY)
{
	// viewport space (2D)			--> normalized device space (3D)
	XMVECTOR dir = XMLoadFloat2(&camera->ScreenToViewport(screenX, screenY));
	dir = XMVectorSetZ(dir, 1.0f);

	// normalized device space (3D)	--> homogeneous clip space (4D)
	dir = XMVectorSetW(dir, 1.0f);	// near plane������ position�� ��� ���̹Ƿ� w = 1

	// homogeneous clip space		--> eye space
	XMMATRIX P = camera->GetProjectionMatrix();
	XMMATRIX InvP = XMMatrixInverse(&XMMatrixDeterminant(P), P);
	dir = XMVector4Transform(dir, InvP);
	dir = XMVectorSetZ(dir, 1.0f);
	dir = XMVectorSetW(dir, 0.0f);	// world space������ ray direction�� ����� ���̹Ƿ� w = 0

	// eye space					--> world space
	XMMATRIX V = camera->GetViewMatrix();
	XMMATRIX InvV = XMMatrixInverse(&XMMatrixDeterminant(V), V);
	dir = XMVector4Transform(dir, InvV);
	dir = XMVector4Normalize(dir);	// z, w ���� �������� �־����Ƿ� normalize�� ���ش�.

	Ray ray;
	ray.position = camera->GetEyePosition();
	XMStoreFloat3(&ray.direction, dir);

	return ray;
}

