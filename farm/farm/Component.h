#pragma once

// 클래스 전방선언
class MeshRenderer;
class BoxCollider;

class Object
{
public:
	virtual void Start() = 0;
	virtual void Update(float dt) = 0;


	bool IsDirty() const { return m_dirty; }
	bool IsActive() const { return m_active; }
	UINT GetBufferID() const { return m_bufferId; }

	void SetDirty(bool value = true) { m_dirty = value; }
	void SetActive(bool value) { m_active = value; }
	void SetBufferId(UINT value) { m_bufferId = value; }

private:
	bool m_active = true;
	UINT m_bufferId = -1;
	bool m_dirty = true;
};

class GameObject : public Object
{
public:
	GameObject();

	virtual void Start();
	virtual void Update(float dt);

	XMMATRIX GetWorldMatrix()
	{
		return XMMATRIX(XMMatrixScaling(m_transform.scale.x, m_transform.scale.y, m_transform.scale.z)
			* XMMatrixRotationRollPitchYaw(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z)
			* XMMatrixTranslation(m_transform.position.x, m_transform.position.y, m_transform.position.z));
	}

	std::string GetName() const { return m_name; }
	std::string GetTag() const { return m_tag; }
	Transform GetTransform() const { return m_transform; }

	void SetName(const std::string& value) { m_name = value; }
	void SetTag(const std::string& value) { m_tag = value; }
	void SetTransform(const Transform& value) { m_transform = value; }

	MeshRenderer* GetRenderer() const { return m_pRenderer.get();	}
	BoxCollider* GetCollider() const { return m_pCollider.get(); }

private:
	Transform m_transform;
	
	std::unique_ptr<MeshRenderer> m_pRenderer;
	std::unique_ptr<BoxCollider> m_pCollider;

	std::string m_name;
	std::string m_tag;
};

class Component 
{
public:
	Component(GameObject* object);

	virtual void Start() = 0;
	virtual void Update(float dt) = 0;

	GameObject* gameObject() { return m_pObject; }

private:
	GameObject* m_pObject;
};

class MeshRenderer : public Component
{
public:
	MeshRenderer(GameObject* object);
	void Start();
	void Update(float dt);

	void SetMesh(std::string name);

	MeshDesc* meshDesc() const { return m_mesh; }

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

	Collider(GameObject* object);
	virtual void Start();
	virtual void Update(float dt);
	
	bool isZero()
	{
		return (m_bound.min.x == 0.0f && m_bound.min.y == 0.0f && m_bound.min.z == 0.0f
			&& m_bound.max.x == 0.0f && m_bound.max.y == 0.0f && m_bound.max.z == 0.0f);
	}

	Bound GetBound() const { return m_bound; }
	XMFLOAT3 GetCenter() const { return m_center; }

protected:
	void SetBound(const Bound& rhs);

private:
	XMFLOAT3 m_center;
	Bound m_bound;
};

class BoxCollider : public Collider
{
public:
	BoxCollider(GameObject* object);
	void Start();
	void Update(float dt);
	void SetBoundFromMesh(MeshDesc* mesh);
};


//------------------------