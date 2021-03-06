#pragma once
#include "DirectXGame.h"

struct Ray
{
	XMFLOAT3 direction;
	XMFLOAT3 position;
};

class Physics
{
public:
	static GameObject* Raycast(Camera* camera, int screenX, int screenY, E_RenderLayer layer = E_RenderLayer::Opaque);

private:
	static Ray GeneratePickingRay(Camera* camera, int screenX, int screenY);
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