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

Player::Player()
{

}

void Player::Start()
{

}

void Player::Update()
{
	
}

void Player::OnMouseMove(WPARAM state, int x, int y)
{
	// todo: 카메라가 가리키는 대상 레이캐스트로 인식
}

void Player::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_pressedW = true;
		break;
	case 'A':
		m_pressedA = true;
		break;
	case 'S':
		m_pressedS = true;
		break;
	case 'D':
		m_pressedD = true;
		break;
	}
}

void Player::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_pressedW = false;
		break;
	case 'A':
		m_pressedA = false;
		break;
	case 'S':
		m_pressedS = false;
		break;
	case 'D':
		m_pressedD = false;
		break;
	}
}