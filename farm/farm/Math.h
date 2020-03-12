#pragma once

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