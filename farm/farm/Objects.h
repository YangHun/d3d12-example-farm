#pragma once
#include "Component.h"


class Plant : public GameObject
{
public:
	Plant();
	void Start();
	void Update(float dt);

private:
	void ReplicateSelf();

public:
	int m_id;
	bool m_replicatable;

private:
	float m_lifeTime;
	float m_timer;

	float m_initScale;
	float m_finalScale;

};

class Field : public GameObject
{
public:
	Field();
	void Start();
	void Update(float dt);

};

class Deer : public GameObject
{
public:
	Deer();

	void Start();
	void Update(float dt);

private:
	float m_angle = 0.0f;
};

class Player : public GameObject
{
public:
	Player();
	void Start();
	void Update(float dt);

	void OnMouseMove(WPARAM state, int x, int y);
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
private:

	bool m_pressedW = false;
	bool m_pressedA = false;
	bool m_pressedS = false;
	bool m_pressedD = false;

};