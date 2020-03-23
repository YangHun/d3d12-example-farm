#include "stdafx.h"
#include "Component.h"
#include "DirectXGame.h"

GameObject::GameObject() :
	m_transform (Transform{}),
	m_pRenderer(std::make_unique<MeshRenderer>(this)),
	m_pCollider(std::make_unique<BoxCollider>(this))
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
	Component(object),
	m_center(object->GetTransform().position)
{
}

void Collider::Start()
{

}

void Collider::Update(float dt)
{

}

void Collider::SetBound(const Bound& rhs)
{
	m_bound = rhs;
}

BoxCollider::BoxCollider(GameObject* object) :
	Collider(object)
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

	Collider::Bound bound;

	bound.min.x = mesh->minBound.x;
	bound.min.y = mesh->minBound.y;
	bound.min.z = mesh->minBound.z;

	bound.max.x = mesh->maxBound.x;
	bound.max.y = mesh->maxBound.y;
	bound.max.z = mesh->maxBound.z;

	SetBound(bound);
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
