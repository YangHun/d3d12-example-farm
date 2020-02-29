#pragma once
interface IObject
{
public:
	virtual void Start() = 0;
	virtual void Update() = 0;
};

class Object : public IObject
{
public:
	Object();

	virtual void Start();
	virtual void Update();

	//void AddComponent(Component* component);
	//void RemoveComponent(Component* component);

public:
	Transform transform;
protected:
	std::vector<IObject*> components;
	bool m_dirty = false;
};

class Component : public IObject
{
public:
	Component();

	virtual void Start() {}
	virtual void Update() {}

	//void Attach(Object* target);
	//void Detach();
protected:
	Object* object = nullptr;
};

class MeshRenderer : public Component
{
public:
	MeshRenderer();
	void Start();
	void Update();

	void SetMesh(std::string name);
protected:
	MeshDesc* mesh = nullptr;

};

//------------ Custom Object

class Deer : public Object
{
public:
	void Start();
	void Update();

private:
	float angle = 0.0f;
};