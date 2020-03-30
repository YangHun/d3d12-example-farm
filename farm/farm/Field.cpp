#include "stdafx.h"
#include "DirectXGame.h"

Plant::Plant() :
	m_lifeTime(30.0f),
	m_growTime(5.0f),
	m_timer(0.0f),
	m_initScale(0.0f),
	m_finalScale(0.005f),
	m_initTransform({ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }),
	m_finalTransform({ {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.01f, 0.01f, 0.01f} }),
	m_state (E_STATE::PLANT_NONE)
{
	GetRenderer()->SetMesh("Assets/carrot.fbx");

	//m_collider = BoxCollider(this);
	//m_collider.SetBound(m_renderer.GetMeshDesc());

	Transform t = GetTransform();
	t.position = m_initTransform.position;
	t.scale = m_initTransform.scale;
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
	t.position.y = m_initTransform.position.y;
	t.scale = m_initTransform.scale;
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
			t.position.y = Math::Lerp(m_initTransform.position.y, m_finalTransform.position.y, scale);
			t.scale = Math::Lerp(m_initTransform.scale,	m_finalTransform.scale, scale);
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
		// ���� �ɰ��� �� ó������ Update() ���� ���� �� state ����
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
	// �� �� �̻� ���� Plant ������Ʈ�� �ٽ� �ʱ�ȭ
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
		// ó������ �Ĺ��� ���� ��, instantiate �Ѵ�			
		m_plant = reinterpret_cast<Plant*>(DirectXGame::GetCurrentScene()->Instantiate<Plant>(E_RenderLayer::Opaque));
		m_plant->SetParent(this);

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