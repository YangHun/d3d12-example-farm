#include "stdafx.h"
#include "DirectXGame.h"

Plant::Plant() :
	m_lifeTime(5.0f),
	m_timer(0.0f),
	m_replicatable(true),
	m_id(1),
	m_initScale(0.0f),
	m_finalScale(0.005f),
	m_state (E_STATE::PLANT_NONE)
{
	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("Assets/plant.fbx");

	//m_collider = BoxCollider(this);
	//m_collider.SetBound(m_renderer.GetMeshDesc());

	m_transform.scale = XMFLOAT3(m_initScale, m_initScale, m_initScale);
	m_active = false;
}


void Plant::Start()
{

}


void Plant::Initialize()
{
	m_lifeTime = 30.0f;
	m_growTime = 5.0f;
	m_timer = 0.0f;	m_id = 1;
	m_initScale = 0.0f;
	m_finalScale = 0.005f;
	m_state = E_STATE::PLANT_NONE;

	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("Assets/plant.fbx");

	//m_collider = BoxCollider(this);
	//m_collider.SetBound(m_renderer.GetMeshDesc());

	m_transform.scale = XMFLOAT3(m_initScale, m_initScale, m_initScale);
	m_active = false;

	//m_replicatable = false;

}

void Plant::Update(float dt)
{
	if (m_timer < m_lifeTime) {
		m_timer += dt;
		
		if (m_timer < m_growTime) {
			float scale = (m_timer / m_growTime);

			m_transform.scale = Math::Lerp(
				XMFLOAT3(m_initScale, m_initScale, m_initScale),
				XMFLOAT3(m_finalScale, m_finalScale, m_finalScale), scale);
			m_dirty = true;
		}		
	}

	UpdateState();
}

void Plant::UpdateState()
{
	switch (m_state)
	{
	case E_STATE::PLANT_READY_TO_GROW:
	{
		// 땅에 심겨진 후 처음으로 Update() 문에 진입 시 state 변경
		m_state = E_STATE::PLANT_GROWING;
		return;
	}
	case E_STATE::PLANT_GROWING:
	{
		if (m_timer >= m_growTime) m_state = E_STATE::PLANT_READY_TO_HARVEST;
		return;
	}
	case E_STATE::PLANT_READY_TO_HARVEST:
	{
		if (m_timer >= m_lifeTime) m_state = E_STATE::PLANT_DEAD;
		return;
	}
	case E_STATE::PLANT_NONE:
	{
		return;
	}
	}
}

void Plant::Seed()
{
	// 한 번 이상 사용된 Plant 오브젝트를 다시 초기화
	Initialize();
	m_active = true;
	m_state = E_STATE::PLANT_READY_TO_GROW;
}

void Plant::Harvest()
{
	m_active = false;
	m_state = E_STATE::PLANT_NONE;

}

// 지금은 안 쓰는 함수
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
	m_tag = "Field";
}

void Field::Start()
{

}

void Field::Update(float dt)
{
	
}

void Field::Interact()
{
	if (m_plant == nullptr)
	{
		// 처음으로 식물을 심을 때, instantiate 한다			
		m_plant = reinterpret_cast<Plant*>(DirectXGame::GetCurrentScene()->Instantiate<Plant>());
		m_plant->m_transform.position = m_transform.position;
		m_plant->m_active = false;
	}
	
	auto state = m_plant->GetCurrentState();

	if (state == Plant::E_STATE::PLANT_NONE && !m_plant->m_active) {
		m_plant->Seed();
	}

	if (state == Plant::E_STATE::PLANT_READY_TO_HARVEST |
		state == Plant::E_STATE::PLANT_DEAD)
	{
		m_plant->Harvest();
	}		
	
}