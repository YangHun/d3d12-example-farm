#pragma once
interface IObject
{
public:
	virtual void Start() = 0;
	virtual void Update() = 0;
};

class Component : public IObject
{
public:
	Component();

	virtual void Start() {}
	virtual void Update() {}

protected:
};

class MeshRenderer : public Component
{
public:
	MeshRenderer();
	void Start();
	void Update();

	void SetMesh(std::string name);
	MeshDesc* m_mesh = nullptr;

};


class Object : public IObject
{
public:
	Object();

	virtual void Start();
	virtual void Update();

	//void AddComponent(Component* component);
	//void RemoveComponent(Component* component);

	bool IsDirty() const { return m_dirty; }
	void SetDirty(bool value) { m_dirty = value; }

public:
	Transform m_transform;
	UINT m_bufferId = -1;

	// components
	MeshRenderer m_renderer;
	//std::unordered_map<std::string, IObject*> components;

	bool m_active = true;

protected:
	bool m_dirty = true;
};



//------------ Custom Object

class Deer : public Object
{
public:
	Deer();

	void Start();
	void Update();

private:
	float m_angle = 0.0f;
};

class Field : public Object
{
public:
	Field();
	void Start();
	void Update();

};

class Player : public Object
{
public:
	Player();
	void Start();
	void Update();

	void OnMouseMove(WPARAM state, int x, int y);
	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
private:
	
	bool m_pressedW = false;
	bool m_pressedA = false;
	bool m_pressedS = false;
	bool m_pressedD = false;

};