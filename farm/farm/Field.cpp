#include "stdafx.h"
#include "Objects.h"


Plant::Plant() :
	m_lifeTime (0.0f),
	m_timer (0.0f)
{
	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("Assets/plant.fbx");

	m_collider = BoxCollider(this);
	m_collider.SetBound(m_renderer.GetMeshDesc());

	m_initialTransform = Transform
	{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.002f, 0.002f, 0.002f}
	};

	m_finalTransform = Transform
	{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.005f, 0.005f, 0.005f}
	};

	m_transform = m_initialTransform;

	m_lifeTime = 5.0f;
}

void Plant::Start()
{

}

void Plant::Update(float dt)
{
	if (!m_active) return;
	if (m_timer > m_lifeTime) return;
	
	m_timer += dt;
	float scale = (m_timer / m_lifeTime);
	
	m_transform.scale = Math::Lerp(m_initialTransform.scale, m_finalTransform.scale, scale);

	m_dirty = true;
}

Field::Field()
{
	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("plane");

	m_collider = BoxCollider(this);
	m_collider.SetBound(m_renderer.GetMeshDesc());
}

void Field::Start()
{

}

void Field::Update(float dt)
{

}