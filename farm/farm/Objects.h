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

class Field : public GameObject, public Subject
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

class Player : public GameObject, public Observer
{
public:
	Player();
	void Start();
	void Update(float dt);

	void OnNotify(Object* object, E_Event event) override;

	void OnMouseMove(WPARAM state, int x, int y);
	void OnMouseDown(WPARAM state, int x, int y);
	void OnMouseUp(WPARAM state, int x, int y);
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);

	int CropCount() { return m_crops; }

	std::wstring PrintPlayerInfo();

private:

	bool m_pressedW = false;
	bool m_pressedA = false;
	bool m_pressedS = false;
	bool m_pressedD = false;

	bool m_leftMouseDown = false;
	bool m_leftMouseUp = false;

	POINT m_cursorPos;


	// data
	int m_crops = 0;
	int m_coins = 0;
};


class HarvestCrate : public GameObject
{
public:
	HarvestCrate();

	void Start();
	void Update(float dt);

	bool IsEmpty() { return m_count < 1; }

	void Increase(int value);
	void Move(Transform transform);

private:
	int m_count;
	int m_size;
	
	std::vector<GameObject*> m_crops;
	bool m_changed;

	const Transform m_idleTransform = Transform
				{ {-5.0f, 0.0f, 15.0f},
				{0.0f, XM_PI / 36.0f, 0.0f},
				{1.0f, 1.0f, 1.0f} };
};

class QuestTable : public GameObject, public Subject
{
public:
	QuestTable();

	void Start();
	void Update(float dt) override;

	void Interact();
};

class InventorySlot : public UIObject
{
public:
	InventorySlot();

	void Start();
	void Update(float dt);

private:
	
};

class Inventory : public UIObject, public Observer
{
public:
	Inventory();

	void Start();
	void Update(float dt);

	void Draw(ID2D1DeviceContext2* pDeviceContext);
	
	void OnNotify(Object* object, E_Event event) override;

private:
	std::vector<InventorySlot*> m_slots;
};