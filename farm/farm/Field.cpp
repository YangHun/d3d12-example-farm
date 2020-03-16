#include "stdafx.h"
#include "DirectXGame.h"

Plant::Plant() :
	m_lifeTime(30.0f),
	m_growTime(5.0f),
	m_timer(0.0f),
	m_initScale(0.0f),
	m_finalScale(0.005f),
	m_state (E_STATE::PLANT_NONE)
{
	GetRenderer()->SetMesh("Assets/carrot.fbx");

	//m_collider = BoxCollider(this);
	//m_collider.SetBound(m_renderer.GetMeshDesc());

	Transform t = GetTransform();
	t.scale = XMFLOAT3(m_initScale, m_initScale, m_initScale);
	SetTransform(t);
	
	SetActive(false);
}


void Plant::Start()
{

}


void Plant::Initialize()
{
	m_timer = 0.0f;
	m_state = E_STATE::PLANT_NONE;


	Transform t = GetTransform();
	t.scale = XMFLOAT3(m_initScale, m_initScale, m_initScale);
	SetTransform(t);

	SetActive(false);


}

void Plant::Update(float dt)
{
	if (m_timer < m_lifeTime) {
		m_timer += dt;
		
		if (m_timer < m_growTime) {
			float scale = (m_timer / m_growTime);

			Transform t = GetTransform();
			t.scale = Math::Lerp(
				XMFLOAT3(m_initScale, m_initScale, m_initScale),
				XMFLOAT3(m_finalScale, m_finalScale, m_finalScale), scale);
			SetTransform(t);

			SetDirty();
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
	SetActive(true);
	m_state = E_STATE::PLANT_READY_TO_GROW;
}

void Plant::Harvest()
{
	SetActive(false);
	m_state = E_STATE::PLANT_NONE;

}

Field::Field() : GameObject()
{
	GetRenderer()->SetMesh("plane");

	MeshDesc* pDesc = GetRenderer()->meshDesc();

	GetCollider()->SetBoundFromMesh(pDesc);
	SetTag("Field");
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

		Transform t = m_plant->GetTransform();
		t.position = GetTransform().position;

		m_plant->SetTransform(t);
		m_plant->SetActive(false);
	}
	
	auto state = m_plant->GetCurrentState();

	if (state == Plant::E_STATE::PLANT_NONE && !m_plant->IsActive()) {
		m_plant->Seed();
	}

	if ( (state == Plant::E_STATE::PLANT_READY_TO_HARVEST) ||
		(state == Plant::E_STATE::PLANT_DEAD))
	{
		if (state == Plant::E_STATE::PLANT_READY_TO_HARVEST) Notify(reinterpret_cast<Object*>(m_plant), E_Event::FIELD_INTERACT_PLANT_HARVEST);
		m_plant->Harvest();

	}		
	
}