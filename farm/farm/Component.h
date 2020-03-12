#pragma once
class Object
{
public:
	virtual void Start() = 0;
	virtual void Update(float dt) = 0;

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
	virtual void Update(float dt) {}

protected:
	Object* m_pObject;
};

class MeshRenderer : public Component
{
public:
	MeshRenderer(Object* object);
	void Start();
	void Update(float dt);

	void SetMesh(std::string name);

	MeshDesc* GetMeshDesc() const
	{
		return m_mesh;
	}

	void SetMaterial(std::string name);
	void SetMaterial(std::string name, std::unique_ptr<Material> pMaterial);

	UINT GetMaterialIndex() const
	{
		if (m_mesh->mesh == nullptr) return 0;
		if (m_mesh->mesh->matIndex.size() < 1) return 0;
		return m_mesh->mesh->matIndex[0];
	}

private:
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
	virtual void SetBound(MeshDesc* mesh) {} ;
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
	virtual void SetBound(MeshDesc* mesh) override;
};

class GameObject : public Object
{
public:
	GameObject();

	virtual void Start();
	virtual void Update(float dt);


public:
	
	// components
	MeshRenderer m_renderer;
	BoxCollider m_collider;
	std::string name = "GameObject";


protected:
};
