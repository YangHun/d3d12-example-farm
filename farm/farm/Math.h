#pragma once

#include <random>

class Math
{
public:
	static float Lerp(float floor, float ceil, float delta)
	{
		delta = Clamp(delta, 0.0f, 1.0f);
		return ((1.0f - delta) * floor) + (delta * ceil);
	}

	static XMFLOAT2 Lerp(XMFLOAT2 floor, XMFLOAT2 ceil, float delta)
	{
		delta = Clamp(delta, 0.0f, 1.0f);
		return XMFLOAT2(
			Lerp(floor.x, ceil.x, delta),
			Lerp(floor.y, ceil.y, delta)
		);
	}

	static XMFLOAT3 Lerp(XMFLOAT3 floor, XMFLOAT3 ceil, float delta)
	{
		delta = Clamp(delta, 0.0f, 1.0f);
		return XMFLOAT3(
			Lerp(floor.x, ceil.x, delta),
			Lerp(floor.y, ceil.y, delta),
			Lerp(floor.z, ceil.z, delta)
		);
	}

	static float Clamp(float value, float floor, float ceil)
	{
		return (value > ceil) ? ceil 
			: (value < floor) ? floor 
			: value;
	}
};

class Random
{
public:
	static bool Boolean()
	{
		static std::random_device rn;
		static std::mt19937_64 rnd(rn());

		std::uniform_int_distribution<int> range(0, 1);

		return static_cast<bool>(range(rnd));
	}

	static int Range(int floor, int ceil)
	{
		static std::random_device rn;
		static std::mt19937_64 rnd(rn());

		std::uniform_int_distribution<int> range(floor, ceil);

		return range(rnd);
	}
	static float Range(float floor, float ceil)
	{
		static std::random_device rn;
		static std::mt19937_64 rnd(rn());

		std::uniform_real_distribution<float> range(floor, ceil);

		return range(rnd);
	}
};