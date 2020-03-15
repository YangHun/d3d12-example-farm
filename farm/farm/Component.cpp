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

void MeshRenderer::SetMaterial(std::string name, std::unique_ptr<Material> pMaterial)
{
	m_mesh->mesh->matIndex.push_back(pMaterial->bufferId);
	Assets::m_materials[name] = std::move(pMaterial);
}

void MeshRenderer::SetMaterial(std::string name)
{
	auto lookup = Assets::m_materials.find(name);
	if (lookup != Assets::m_materials.end()) m_mesh->mesh->matIndex.push_back(lookup->second->bufferId);	
}

void MeshRenderer::SetMesh(std::string name)
{
	auto lookup = Assets::m_meshes.find(name);

	if (lookup != Assets::m_meshes.end()) m_mesh = lookup->second.get();
	else m_mesh = nullptr;
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

