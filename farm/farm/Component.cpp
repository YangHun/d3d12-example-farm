#include "stdafx.h"
#include "Component.h"
#include "DirectXGame.h"

Object::Object()
{
}

void Object::Start()
{
	/*for (auto& component : components)
	{
		component.second->Start();
	}*/
}

void Object::Update()
{
	/*for (auto& component : components)
	{
		component.second->Update();
	}*/
}

Component::Component()
{
}

MeshRenderer::MeshRenderer()
{
}

void MeshRenderer::Start()
{
}

void MeshRenderer::Update()
{

}

void MeshRenderer::SetMesh(std::string name)
{
	auto lookup = Assets::m_meshes.find(name);

	if (lookup != Assets::m_meshes.end()) m_mesh = lookup->second.get();
	else m_mesh = nullptr;
}

Deer::Deer()
{
	m_renderer = MeshRenderer();
	m_renderer.SetMesh("Assets/deer.fbx");
}

void Deer::Start() {
	
}

void Deer::Update() {
	m_angle += 0.001f;
	m_transform.rotation.y = XM_PI * 0.3f + m_angle;
	m_dirty = true;
}

Field::Field()
{
	m_renderer = MeshRenderer();
	m_renderer.SetMesh("plain");
}

void Field::Start()
{

}

void Field::Update()
{

}