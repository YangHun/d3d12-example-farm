#pragma once
#include "D3DGameEngine.h"

namespace Assets
{
	const UINT numScenes = 2;


	const std::vector<std::vector<Vertex>> triangleVertices =
	{
		{
			{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		},
		{
			{ { 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ { 0.25f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }
		}
	};


	const std::vector<uint16_t> triangleIndices =
	{
		0, 1, 2
	};


}