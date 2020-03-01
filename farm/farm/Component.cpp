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
	mesh = pMesh;
}

Deer::Deer()
{
	renderer = MeshRenderer();
	//components.push_back(&renderer);
	//renderer.SetMesh("Assets/deer.fbx");
	
	//transform.position = XMFLOAT3{ 0.0f, -1.0f, 0.0f };
	//transform.rotation = XMFLOAT3{ 0.0f, XM_PI * 0.3f, 0.0f };
	//transform.scale = XMFLOAT3{ 0.002f, 0.002f, 0.002f };
}

void Deer::Start() {
	
}

void Deer::Update() {
	angle += 0.001f;
	transform.rotation.y = XM_PI * 0.3f + angle;
	m_dirty = true;
}