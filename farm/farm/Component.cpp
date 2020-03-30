#include "stdafx.h"
#include "DirectXGame.h"
#include "Component.h"
#include "Physics.h"

GameObject::GameObject() :
	m_transform (Transform{}),
	m_pRenderer(std::make_unique<MeshRenderer>(this)),
	m_pCollider(std::make_unique<BoxCollider>(this)),
	m_layer(0)
{
	
}

void GameObject::Start()
{
	// Do I really need this method or not?
}

void GameObject::Update(float dt)
{
	m_pRenderer->Update(dt);
}

XMMATRIX GameObject::GetWorldMatrix()
{
	XMMATRIX world = XMMATRIX(XMMatrixScaling(m_transform.scale.x, m_transform.scale.y, m_transform.scale.z)
		* XMMatrixRotationRollPitchYaw(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z)
		* XMMatrixTranslation(m_transform.position.x, m_transform.position.y, m_transform.position.z));

	GameObject* parent = reinterpret_cast<GameObject*>(GetParentObject());
	if (parent != nullptr) world = world * parent->GetWorldMatrix();

	return world;
}

Component::Component(GameObject* object) :
	m_pObject(object)
{
}

MeshRenderer::MeshRenderer(GameObject* object) : Component(object)
{
}

void MeshRenderer::Start()
{
}

void MeshRenderer::Update(float dt)
{

}

Material* MeshRenderer::GetMaterial()
{
	if (m_material == nullptr) m_material = Assets::GetMaterialByID(GetMaterialIndex());
	return m_material;
}

void MeshRenderer::SetMaterial(std::string name, std::unique_ptr<Material> pMaterial)
{
	m_material = pMaterial.get();
	Assets::m_materials[name] = std::move(pMaterial);
}

void MeshRenderer::SetMaterial(std::string name)
{
	auto lookup = Assets::m_materials.find(name);
	if (lookup != Assets::m_materials.end()) {
		m_material = lookup->second.get();
	}
}

void MeshRenderer::AssignInstance(const InstanceData& data)
{
	if (m_mesh == nullptr) return;
	m_mesh->instanceBuffer[gameObject()->GetBufferID()] = data;
}

void MeshRenderer::SetMesh(std::string name)
{
	auto lookup = Assets::m_meshes.find(name);

	if (lookup != Assets::m_meshes.end()) {
		m_mesh = lookup->second.get();
		
		if (m_mesh->mesh != nullptr && !m_mesh->mesh->matIndex.empty()) m_material = Assets::GetMaterialByID(m_mesh->mesh->matIndex[0]);
	}
	else
	{
		m_mesh = nullptr;
		m_material = nullptr;
	}
}

Collider::Collider(GameObject* object) :
	Component(object)
{
}

BoxCollider::BoxCollider(GameObject* object) :
	Collider(object),
	m_bound({ 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 0.0f})
{
}


void BoxCollider::Start()
{

}

void BoxCollider::Update(float dt)
{

}

void BoxCollider::SetBoundFromMesh(MeshDesc* desc)
{
	if (desc == nullptr) return;
	Mesh* mesh = desc->mesh;

	XMVECTOR _minBound = XMLoadFloat3(&mesh->minBound);
	XMVECTOR _maxBound = XMLoadFloat3(&mesh->maxBound);

	BoundingBox _bound;

	XMStoreFloat3(&_bound.Center, XMVectorAdd(_minBound, _maxBound) / 2.0f);
	XMStoreFloat3(&_bound.Extents, XMVectorSubtract(_maxBound, _minBound) / 2.0f);

	SetBound(_bound);
}

bool IsExist(const XMFLOAT3& value, const XMFLOAT3& floor, const XMFLOAT3& ceil)
{
	return (value.x >= floor.x) && (value.x <= ceil.x)
		&& (value.y >= floor.y) && (value.y <= ceil.y)
		&& (value.z >= floor.z) && (value.z <= ceil.z);
}

bool BoxCollider::IsZero()
{
	XMFLOAT3 _c, _e;
	
	XMStoreFloat3(&_c, XMVectorEqual(XMLoadFloat3(&m_bound.Center), DirectX::XMVectorZero()));
	XMStoreFloat3(&_e, XMVectorEqual(XMLoadFloat3(&m_bound.Extents), DirectX::XMVectorZero()));

	// XMVectorEqual() 결과값으로 요소가 동일하지 않을 경우, 0 반환
	// https://docs.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmvectorgreaterorequal
	return _c.x != 0.0f && _c.y != 0.0f && _c.z != 0.0f
		&& _e.x != 0.0f && _e.y != 0.0f && _e.z != 0.0f;

}

float BoxCollider::Pick(Ray ray)
{
	XMVECTOR _extent = XMLoadFloat3(&m_bound.Extents);
	XMVECTOR _center = XMLoadFloat3(&m_bound.Center);

	XMFLOAT3 _min, _max; 
	XMStoreFloat3(&_min, XMVectorSubtract(_center, _extent));
	XMStoreFloat3(&_max, XMVectorAdd(_center, _extent));

	// base case : P가 bounding box 내부에 위치할 경우, true
	if (IsExist(ray.position, _min, _max)) return 0.1f;

	// AABB 체크
	// 직선 상의 점은 _pos + k * _dir (k is constant) 로 나타낼 수 있다.
	// x, y, z축의 최소/최대 값 bound와 직선이 만나는 6개의 점에 대한 k 값을 각각 구한다.
	float v[6];
	v[0] = (ray.direction.x != 0 ? ((_min.x - ray.position.x) / ray.direction.x) : FLT_MAX);
	v[1] = (ray.direction.x != 0 ? ((_max.x - ray.position.x) / ray.direction.x) : FLT_MAX);
	v[2] = (ray.direction.y != 0 ? ((_min.y - ray.position.y) / ray.direction.y) : FLT_MAX);
	v[3] = (ray.direction.y != 0 ? ((_max.y - ray.position.y) / ray.direction.y) : FLT_MAX);
	v[4] = (ray.direction.z != 0 ? ((_min.z - ray.position.z) / ray.direction.z) : FLT_MAX);
	v[5] = (ray.direction.z != 0 ? ((_max.z - ray.position.z) / ray.direction.z) : FLT_MAX);

	// near	: 축별로 P와 가까운 점 3개(x_near, y_near, z_near)의 k값 중 최대값
	// far	: 축별로 P와 멀리 있는 점 3개(x_far, y_far, z_far)의 k값 중 최소값
	float _near, _far;
	_near = Max((v[0] < FLT_MAX ? 1.0f : -1.0f) * min(v[0], v[1]),
		(v[2] < FLT_MAX ? 1.0f : -1.0f) * min(v[2], v[3]),
		(v[4] < FLT_MAX ? 1.0f : -1.0f) * min(v[4], v[5]));
	_far = Min(max(v[0], v[1]), max(v[2], v[3]), max(v[4], v[5]));

	return ((_far >= 0) && (_near <= _far)) ? _near : FLT_MAX;
}

void BoxCollider::SetBound(const BoundingBox& value)
{
	m_bound = value;

#ifdef COLLIDER_DEBUG
	if (m_pBox == nullptr)
	{
		m_pBox = DirectXGame::GetCurrentScene()->Instantiate<GameObject>("collider", Transform{ {0.0f, 0.0f, 0.0}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} }, true, E_RenderLayer::ColliderDebug);
		m_pBox->SetParent(gameObject());
		m_pBox->GetRenderer()->SetMesh("box");
	}
	Transform t = m_pBox->GetTransform();
		
	t.position = value.Center;
		
	XMVECTOR extents = XMLoadFloat3(&value.Extents);
	XMStoreFloat3(&t.scale, extents * 2.0f);

	m_pBox->SetTransform(t);
	
#endif
}

void BoxCollider::SetCenter(const XMFLOAT3& value)
{
	m_bound.Center = value;

#ifdef COLLIDER_DEBUG
	if (m_pBox != nullptr)
	{
		Transform t = m_pBox->GetTransform();
		t.position = value;
		m_pBox->SetTransform(t);
	}
#endif
}

//-------------------- ui objects

UIObject::UIObject() :
	m_pSprite(std::make_unique<SpriteRenderer>(this)),
	m_pText(std::make_unique<UIText>(this))
{

}

void UIObject::Start()
{

}

void UIObject::Update(float dt)
{

}

void UIObject::Draw(ID2D1DeviceContext2* pDeviceContext)
{
	m_pSprite->Draw(pDeviceContext);
	m_pText->Draw(pDeviceContext);
}

UIComponent::UIComponent(UIObject* object) :
	m_pObject (object)
{

}

SpriteRenderer::SpriteRenderer(UIObject* object) : UIComponent(object),
	m_rect(Rect{0.0f, 0.0f, 0.0f, 0.0f})
{

}

void SpriteRenderer::Start()
{}

void SpriteRenderer::Update(float dt)
{}

void SpriteRenderer::SetSprite(std::string name)
{
	auto lookup = Assets::m_sprites.find(name);
	if (lookup != Assets::m_sprites.end()) m_pSprite = lookup->second.get();
	else m_pSprite = nullptr;
}

void SpriteRenderer::SetRect(float x, float y)
{
	auto size = m_pSprite->bitmap.Get()->GetSize();
	m_rect.left = x;
	m_rect.top = y;
	m_rect.right = x + size.width;
	m_rect.bottom = y + size.height;
}

void SpriteRenderer::Draw(ID2D1DeviceContext2* pDeviceContext)
{
	if (m_pSprite == nullptr) return;

	D2D1_RECT_F rect = D2D1::RectF(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	
	pDeviceContext->DrawBitmap(
		m_pSprite->bitmap.Get(),
		&rect,
		m_alpha,
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
		NULL
	);
}

UIText::UIText(UIObject* object) : UIComponent(object),
	m_pDesc (nullptr),
	m_rect (Rect{0.0f, 0.0f, 0.0f, 0.0f})
{

}

void UIText::SetTextDesc(std::string name)
{
	auto lookup = Assets::m_texts.find(name);
	if (lookup != Assets::m_texts.end()) m_pDesc = lookup->second.get();
	else m_pDesc = nullptr;
}

void UIText::Start()
{

}

void UIText::Update(float dt)
{

}

void UIText::Draw(ID2D1DeviceContext2* pDeviceContext)
{
	if (m_pDesc == nullptr) return;

	D2D1_RECT_F rect = D2D1::RectF(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	pDeviceContext->DrawTextW
	(
		m_text.c_str(),
		m_text.size(),
		m_pDesc->textFormat.Get(),
		&rect,
		m_pDesc->brush.Get()
	);
}

Observer::Observer()
{

}
