#pragma once
class Object
{
public:
	virtual void Start() = 0;
	virtual void Update() = 0;

	XMMATRIX GetWorldMatrix() const
	{
		return XMMatrixScaling(m_transform.scale.x, m_transform.scale.y, m_transform.scale.z)
			* XMMatrixRotationRollPitchYaw(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z)
			* XMMatrixTranslation(m_transform.position.x, m_transform.position.y, m_transform.position.z);
	}

	bool IsDirty() const { return m_dirty; }
	void SetDirty(bool value) { m_dirty = value; }

public:
	Transform m_transform;
	bool m_active = true;
	UINT m_bufferId = -1;

protected:

	bool m_dirty = true;
};

class Component 
{
public:
	Component(Object* object);

	virtual void Start() {}
	virtual void Update() {}

protected:
	Object* m_pObject;
};

class MeshRenderer : public Component
{
public:
	MeshRenderer(Object* object);
	void Start();
	void Update();

	void SetMesh(std::string name);
	MeshDesc* m_mesh = nullptr;

};

class Collider : public Component
{
public:
	struct Bound
	{
		XMFLOAT3 min = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 max = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	Collider(Object* object);
	virtual void SetBound(Mesh* mesh) {} ;
	bool IsZeroBound()
	{
		return (m_bound.min.x == 0.0f && m_bound.min.y == 0.0f && m_bound.min.z == 0.0f
			&& m_bound.max.x == 0.0f && m_bound.max.y == 0.0f && m_bound.max.z == 0.0f);
	}

public:
	XMFLOAT3 m_center;
	Bound m_bound;
};

class BoxCollider : public Collider
{
public:
	BoxCollider(Object* object);
	virtual void SetBound(Mesh* mesh) override;
};

class GameObject : public Object
{
public:
	GameObject();

	virtual void Start();
	virtual void Update();

public:
	
	// components
	MeshRenderer m_renderer;
	BoxCollider m_collider;
	std::string name = "GameObject";


protected:
};




//------------ Custom Object

class Deer : public GameObject
{
public:
	Deer();

	void Start();
	void Update();

private:
	float m_angle = 0.0f;
};

class Field : public GameObject
{
public:
	Field();
	void Start();
	void Update();

};

class Player : public GameObject
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