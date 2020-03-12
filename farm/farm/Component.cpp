#include "stdafx.h"
#include "Component.h"
#include "DirectXGame.h"

GameObject::GameObject() :
	m_renderer(this),
	m_collider(this)
{
	
}

void GameObject::Start()
{
	// Do I really need this method or not?
}

void GameObject::Update(float dt)
{
	m_renderer.Update(dt);
}


Component::Component(Object* object) :
	m_pObject(object)
{
}

MeshRenderer::MeshRenderer(Object* object) : Component(object)
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


Collider::Collider(Object* object) : 
	Component(object),
	m_center(object->m_transform.position)
{
}

BoxCollider::BoxCollider(Object* object) :
	Collider(object)
{
}

void BoxCollider::SetBound(MeshDesc* desc)
{
	if (desc == nullptr) return;
	Mesh* mesh = desc->mesh;

	m_bound.min.x = mesh->minBound.x;
	m_bound.min.y = mesh->minBound.y;
	m_bound.min.z = mesh->minBound.z;

	m_bound.max.x = mesh->maxBound.x;
	m_bound.max.y = mesh->maxBound.y;
	m_bound.max.z = mesh->maxBound.z;
}
