#pragma once
#include "Component.h"
#include <queue>

class Plant : public GameObject
{
public:
	enum class E_STATE 
	{
		PLANT_READY_TO_SEED,
		PLANT_READY_TO_GROW,
		PLANT_GROWING,
		PLANT_READY_TO_HARVEST,
		PLANT_DEAD,
		COUNT
	};

	Plant();
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
	
	Transform m_initTransform;
	Transform m_finalTransform;
	float m_initScale;
	float m_finalScale;

	E_STATE m_state;
};

class Field : public GameObject, public Subject
{
public:
	enum class E_STATE
	{
		FIELD_IDLE,
		FIELD_READY_TO_AUTO_GROW,
		FIELD_OCCUPIED,
		FIELD_READY_TO_IDLE,
		COUNT
	};

	Field();
	void Update(float dt);

	void Interact();

private:
	void UpdateState();

private:
	Plant* m_plant;
	float m_timer;
	float m_idleTime;
	E_STATE m_state;
};


class Bed : public GameObject
{
public:
	Bed();
	void Update(float dt);

	void Interact();

};

class Tree : public GameObject
{
public:
	Tree();
	void Update(float dt);
};

class Player : public GameObject, public Observer
{
public:
	Player();
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

	void Update(float dt);
	bool IsEmpty() { return m_count < 1; }
	bool IsFull() { return m_count >= m_size; }

	void Increase(int value);
	void MoveNearObject(GameObject* object);

private:
	int m_count;
	int m_size;
	
	std::vector<GameObject*> m_crops;
	bool m_changed;

	const Transform m_idleTransform = Transform
				{ {-5.0f, 0.0f, 15.0f},
				{0.0f, XM_PI / 36.0f, 0.0f},
				{1.0f, 1.0f, 1.0f} };

	const XMFLOAT3 localPivot = { -1.8f, 0.0f, -3.8f };
};

class CrateManager : public Object, public Observer
{
public:
	CrateManager();
	void Update(float dt);

	void OnNotify(Object* object, E_Event event) override;

private:
	HarvestCrate* Spawn(Transform transform);
	void Stack(HarvestCrate* target);

private:
	std::queue<GameObject*> m_pool;
	HarvestCrate* m_current;
	Transform m_spawnPoint;
	int m_spawnCount;
};


class QuestTable : public GameObject, public Subject
{
public:
	QuestTable();

	void Update(float dt) override;

	void Interact();
};

class InventorySlot : public UIObject
{
public:
	InventorySlot();

	void Update(float dt);

private:
	
};

class Inventory : public UIObject, public Observer
{
public:
	Inventory();

	void Update(float dt);

	void Draw(ID2D1DeviceContext2* pDeviceContext) override;
	
	void OnNotify(Object* object, E_Event event) override;

private:
	std::vector<InventorySlot*> m_slots;
};

class QuestInfo : public UIObject, public Observer
{
public:
	QuestInfo();

	void Update(float dt);
	void Draw(ID2D1DeviceContext2* pDeviceContext) override;
	void OnNotify(Object* object, E_Event event) override;

private:
	std::unique_ptr<SpriteRenderer> m_pBackground;
};