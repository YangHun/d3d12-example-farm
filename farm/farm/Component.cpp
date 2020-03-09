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

void GameObject::Update()
{
	m_renderer.Update();
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

void MeshRenderer::Update()
{

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

void BoxCollider::SetBound(Mesh* mesh)
{
	m_bound.min.x = mesh->minBound.x;
	m_bound.min.y = mesh->minBound.y;
	m_bound.min.z = mesh->minBound.z;

	m_bound.max.x = mesh->maxBound.x;
	m_bound.max.y = mesh->maxBound.y;
	m_bound.max.z = mesh->maxBound.z;

	/*m_bound.y.x = mesh->yBound.x;
	m_bound.y.y = mesh->yBound.y;

	m_bound.z.x = mesh->zBound.x;
	m_bound.z.y = mesh->zBound.y;*/

	//m_bound.x = mesh->xBound;
	//m_bound.y = mesh->yBound;
	//m_bound.x = mesh->zBound;
}

Deer::Deer()
{
	m_renderer = MeshRenderer(this);
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
	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("plain");
	m_collider = BoxCollider(this);
	m_collider.SetBound(m_renderer.m_mesh->mesh);
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
