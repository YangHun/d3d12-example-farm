#include "stdafx.h"
#include "Physics.h"

GameObject* Physics::Raycast(Camera* camera, int screenX, int screenY, E_RenderLayer layer)
{
	// generate the picking ray vector.
	Ray ray = GeneratePickingRay(camera, screenX, screenY);
	//Ray ray = GeneratePickingRay(camera, 640, 360);	// 중앙

	// raycast using Axis Aligned Bounding Boxes(AABB).
	auto scene = DirectXGame::GetCurrentScene();
	float distance = 1000.0f;

	GameObject* hit = nullptr;
	float min_distance = FLT_MAX;


	for (auto obj : scene->GetObjectsByLayer(layer))
	{
		if (!obj->IsActive()) continue;
		
		if (obj->GetCollider()->IsZero()) continue; // not have bounding box.
		
		// bounding box를 가진 object가 world transform을 가질 수 있으므로
		// ray에 object의 (W)^-1 을 곱한 다음 충돌 여부를 체크한다.
		
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

Ray Physics::GeneratePickingRay(Camera* camera, int screenX, int screenY)
{
	// viewport space (2D)			--> normalized device space (3D)
	XMVECTOR dir = XMLoadFloat2(&camera->ScreenToViewport(screenX, screenY));
	dir = XMVectorSetZ(dir, 1.0f);

	// normalized device space (3D)	--> homogeneous clip space (4D)
	dir = XMVectorSetW(dir, 1.0f);	// near plane에서의 position을 계산 중이므로 w = 1

	// homogeneous clip space		--> eye space
	XMMATRIX P = camera->GetProjectionMatrix();
	XMMATRIX InvP = XMMatrixInverse(&XMMatrixDeterminant(P), P);
	dir = XMVector4Transform(dir, InvP);
	dir = XMVectorSetZ(dir, 1.0f);
	dir = XMVectorSetW(dir, 0.0f);	// world space에서의 ray direction을 계산할 것이므로 w = 0

	// eye space					--> world space
	XMMATRIX V = camera->GetViewMatrix();
	XMMATRIX InvV = XMMatrixInverse(&XMMatrixDeterminant(V), V);
	dir = XMVector4Transform(dir, InvV);
	dir = XMVector4Normalize(dir);	// z, w 값을 수동으로 넣었으므로 normalize를 해준다.

	Ray ray;
	ray.position = camera->GetEyePosition();
	XMStoreFloat3(&ray.direction, dir);

	return ray;
}

