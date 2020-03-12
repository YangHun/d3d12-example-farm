#pragma once
#include "DirectXGame.h"
#include "Component.h"
#include "Camera.h"
#include <algorithm>

class Physics
{
	struct Ray
	{
		XMFLOAT3 direction;
		XMFLOAT3 position;
	};

public:
	static GameObject* Raycast(Camera camera, int screenX, int screenY);

private:
	static Ray GeneratePickingRay(Camera camera, int screenX, int screenY);
	static float PickAABB(Ray ray, Collider::Bound bound);
};


// helper functions

inline float Max(float a, float b, float c)
{
	return max(max(a, b), c);
}

inline float Min(float a, float b, float c)
{
	return min(min(a, b), c);
}

inline bool IsExist(float value, float floor, float ceil)
{
	return (value >= floor) && (value <= ceil);
}