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

void MeshRenderer::SetMesh(MeshDesc* pMesh)
{
	m_mesh = pMesh;
}

Deer::Deer()
{
	m_renderer = MeshRenderer();
	//components.push_back(&renderer);
	//renderer.SetMesh("Assets/deer.fbx");
	
	//transform.position = XMFLOAT3{ 0.0f, -1.0f, 0.0f };
	//transform.rotation = XMFLOAT3{ 0.0f, XM_PI * 0.3f, 0.0f };
	//transform.scale = XMFLOAT3{ 0.002f, 0.002f, 0.002f };
}

void Deer::Start() {
	
}

void Deer::Update() {
	m_angle += 0.001f;
	m_transform.rotation.y = XM_PI * 0.3f + m_angle;
	m_dirty = true;
}