#include "stdafx.h"
#include "Objects.h"

#include "DirectXGame.h"

Plant::Plant() :
	m_lifeTime(5.0f),
	m_timer(0.0f),
	m_replicatable(true),
	m_id(1),
	m_initScale(0.0f),
	m_finalScale(0.005f)
{
	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("Assets/plant.fbx");

	m_collider = BoxCollider(this);
	m_collider.SetBound(m_renderer.GetMeshDesc());

	m_transform.scale = XMFLOAT3(m_initScale, m_initScale, m_initScale);
}


void Plant::Start()
{

}

void Plant::Update(float dt)
{
	if (!m_active) return;
	if (m_timer > m_lifeTime) {
		//if (m_replicatable) ReplicateSelf();
		return;
	}
	
	m_timer += dt;
	float scale = (m_timer / m_lifeTime);
	
	m_transform.scale = Math::Lerp(
		XMFLOAT3(m_initScale, m_initScale, m_initScale), 
		XMFLOAT3(m_finalScale, m_finalScale, m_finalScale), scale);

	m_dirty = true;
}

void Plant::ReplicateSelf()
{
	m_replicatable = false;

	for (int i = 0; i < 50; ++i) {

		Transform t = m_transform;
		t.position.x += Random::Range(1.0f, 2.0f) * (Random::Boolean() ? 1.0f : -1.0f);
		t.position.z += Random::Range(1.0f, 2.0f) * (Random::Boolean() ? 1.0f : -1.0f);


		Plant* replica = reinterpret_cast<Plant*>(DirectXGame::GetCurrentScene()->Instantiate<Plant>());
		replica->m_transform.position = t.position;
		replica->m_id = m_id * 2 + i;
		replica->m_lifeTime = Random::Range(4.0f, 6.0f);
		replica->m_name = "Plant_" + std::to_string(replica->m_id);
		//replica->m_replicatable = (replica->m_id > 128) ? false : true;
		replica->m_replicatable = false;
		replica->m_active = true;
	}
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