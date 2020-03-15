#pragma once
#include "Component.h"

class Plant : public GameObject
{
public:
	enum class E_STATE 
	{
		PLANT_NONE,
		PLANT_READY_TO_GROW,
		PLANT_GROWING,
		PLANT_READY_TO_HARVEST,
		PLANT_DEAD,
		COUNT
	};

	Plant();
	void Start();
	void Update(float dt);

	E_STATE GetCurrentState() const { return m_state; }
	void Seed();
	void Harvest();

private:
	void UpdateState();
	void Initialize();

private:
	float m_lifeTime;
	float m_growTime;
	float m_timer;
	float m_initScale;
	float m_finalScale;

	E_STATE m_state;
};

class Field : public GameObject
{
public:
	Field();
	void Start();
	void Update(float dt);

	void Interact();

private:
	Plant* m_plant = nullptr;
};


class Bed : public GameObject
{
public:
	Bed();
	void Start();
	void Update(float dt);

	void Interact();

};

class Player : public GameObject
{
public:
	Player();
	void Start();
	void Update(float dt);

	void OnMouseMove(WPARAM state, int x, int y);
	void OnMouseDown(WPARAM state, int x, int y);
	void OnMouseUp(WPARAM state, int x, int y);
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
private:

	bool m_pressedW = false;
	bool m_pressedA = false;
	bool m_pressedS = false;
	bool m_pressedD = false;

	bool m_leftMouseDown = false;
	bool m_leftMouseUp = false;

	POINT m_cursorPos;

};
