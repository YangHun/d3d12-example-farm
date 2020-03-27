#pragma once
#include "FrameResource.h"

// 클래스 전방선언
class GameObject;
class UIObject;

class Object
{
public:
	virtual void Start() = 0;
	virtual void Update(float dt) = 0;

	bool IsDirty()  
	{ 
		return m_dirty;
	}
	bool IsActive() 
	{
		bool parent = m_parent != nullptr ? m_parent->IsActive() : true;
		return parent && m_active; 
	}

	UINT GetBufferID() { return m_bufferId; }

	void SetDirty(bool value = true) 
	{ 
		// 값이 true로 바뀔 경우, 자식의 Dirty flag를 true 처리.
		m_dirty = value;
		if (value == true)
		{
			for (auto& i : m_children) i->SetDirty(true);
		}
	}

	void SetActive(bool value) 
	{ 
		m_active = value; 
		SetDirty(true);
	}
	void SetBufferId(UINT value) { m_bufferId = value; }

	void SetParent(Object* value) 
	{ 
		if (m_parent == value) return;
		m_parent = value; 
		value->SetChild(this);
		SetDirty(true);
	}
	void DetachParent() 
	{ 
		if (m_parent == nullptr) return;
		Object* _parent = m_parent;
		m_parent = nullptr; 
		_parent->DetachChild(this);
		SetDirty(true);
	}
	
	void SetChild(Object* value)
	{
		if (std::find(m_children.begin(), m_children.end(), value) != m_children.end()) return;
		m_children.push_back(value);
		value->SetParent(this);
	}

	void DetachChild(Object* value)
	{
		auto lookup = std::find(m_children.begin(), m_children.end(), value);
		if (lookup == m_children.end()) return;
		m_children.erase(lookup);
		value->DetachParent();
	}

	Object* GetParentObject() { return m_parent; }

private:
	Object* m_parent;
	std::vector<Object*> m_children;
	bool m_active = true;
	UINT m_bufferId = -1;
	bool m_dirty = true;
};

class Component 
{
public:
	Component(GameObject* object);

	virtual void Start() = 0;
	virtual void Update(float dt) = 0;

	GameObject* gameObject() const { return m_pObject; }

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
		if (m_mesh == nullptr) return 0;
		if (m_mesh->mesh == nullptr) return 0;
		if (m_mesh->mesh->matIndex.size() < 1) return 0;
		return m_material->bufferId;
	}

	Material* GetMaterial();

	void AssignInstance(const InstanceData& data);
	//void UpdateInstance(const InstanceData& data);

private:
	MeshDesc* m_mesh = nullptr;
	Material* m_material = nullptr;
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
	
	virtual void SetBound(const Bound& value) { m_bound = value; }
	virtual void SetCenter(const XMFLOAT3& value) { m_center = value; }


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


#ifdef COLLIDER_DEBUG
	GameObject* GetBoundBoxObject() const { return m_pBox; }
	
	using Collider::SetBound;
	void SetBound(const Bound& value) override;
	
	using Collider::SetCenter;
	void SetCenter(const XMFLOAT3& value) override;

private:
	GameObject* m_pBox = nullptr;

#endif // COLLIDER_DEBUG
};


class UIComponent
{
public:
	UIComponent(UIObject* object);

	virtual void Start() = 0;
	virtual void Update(float dt) = 0;

	virtual void Draw(ID2D1DeviceContext2*) = 0;

	UIObject* uiObject() { return m_pObject; }

private:
	UIObject* m_pObject;
};


class UIText : public UIComponent
{
public:
	UIText(UIObject* object);

	void Start();
	void Update(float dt);

	void Draw(ID2D1DeviceContext2* pDeviceContext);

	void SetText(const std::wstring value) { m_text = value; }
	void SetRect(const Rect value) { m_rect = value; }
	void SetTextDesc(std::string name);

	Rect GetRect() { return m_rect; }
	std::wstring GetText() const { return m_text; }
	//TextDesc* GetTextDesc() const { return m_pDesc; }

private:
	std::wstring m_text;
	TextDesc* m_pDesc;
	Rect m_rect;
};


class SpriteRenderer : public UIComponent
{
public:
	SpriteRenderer(UIObject* object);
	void Start();
	void Update(float dt);

	void Draw(ID2D1DeviceContext2* pDeviceContext);

	void SetAlpha(float value) { m_alpha = value; }
	void SetRect(const Rect value) { m_rect = value; }
	void SetRect(float x, float y);
	void SetSprite(std::string name);

	float GetAlpha() { return m_alpha; }
	Rect GetRect() { return m_rect; }
	Sprite* GetSprite() const { return m_pSprite; }
	UINT GetBufferId() const
	{
		if (m_pSprite == nullptr) return 0;
		return m_pSprite->id;
	}

private:
	Sprite* m_pSprite = nullptr;
	Rect m_rect;
	float m_alpha = 1.0f;
};


//------------------------

enum class E_Event
{
	FIELD_INTERACT_PLANT_HARVEST,

	TABLE_INTERACT_CLICKED,
};

class Observer
{
public:
	Observer();
	virtual void OnNotify(Object* object, E_Event event) = 0;
};

class Subject
{
public:
	void AddObserver(Observer* observer)
	{
		m_observers.push_back(observer);
	}

protected:
	void Notify(Object* object, E_Event event)
	{
		for (auto o : m_observers)
		{
			o->OnNotify(object, event);
		}
	}

private:
	std::vector<Observer*> m_observers;
};

//------------------------

class GameObject : public Object
{
public:
	GameObject();

	virtual void Start();
	virtual void Update(float dt);

	XMMATRIX GetWorldMatrix();

	std::string GetName() { return m_name; }
	std::string GetTag() { return m_tag; }
	Transform GetTransform() { return m_transform; }
	int GetLayer() { return m_layer; }
	UINT GetInstanceID() { return m_instanceID; }

	void SetName(const std::string& value) { m_name = value; }
	void SetTag(const std::string& value) { m_tag = value; }
	void SetTransform(const Transform& value) 
	{ 
		m_transform = value; 
		SetDirty(true);
	}
	void SetLayer(int value) { m_layer = value; }
	void SetInstanceID(UINT value) { m_instanceID = value; }

	MeshRenderer* GetRenderer() const { return m_pRenderer.get(); }
	BoxCollider* GetCollider() const { return m_pCollider.get(); }

private:
	Transform m_transform;

	std::unique_ptr<MeshRenderer> m_pRenderer;
	std::unique_ptr<BoxCollider> m_pCollider;

	std::string m_name;
	std::string m_tag;

	int m_layer;
	UINT m_instanceID;
};

class UIObject : public Object
{
public:
	UIObject();

	virtual void Start();
	virtual void Update(float dt);

	virtual void Draw(ID2D1DeviceContext2* pDeviceContext);

	void SetName(const std::string& value) { m_name = value; }
	void SetTag(const std::string& value) { m_tag = value; }

	SpriteRenderer* GetSpriteRenderer() const { return m_pSprite.get(); }
	UIText* GetUIText() const { return m_pText.get(); }

private:
	//Transform m_transform;

	std::unique_ptr<SpriteRenderer> m_pSprite;
	std::unique_ptr<UIText> m_pText;

	std::string m_name;
	std::string m_tag;
};

