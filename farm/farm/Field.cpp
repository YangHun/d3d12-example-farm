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
	m_state (E_STATE::PLANT_READY_TO_SEED)
{
	GetRenderer()->SetMesh("Assets/carrot.fbx");
	GetRenderer()->SetMaterial("carrot-mat");

	float random = Random::Range(0.0f, 0.5f);
	m_finalTransform.scale = { random * 0.001f + 0.01f , random * 0.001f + 0.01f , random * 0.001f + 0.01f };
	m_finalTransform.position.y = random * -0.5f - 1.0f;

	Transform t = GetTransform();
	t.position = m_initTransform.position;
	t.rotation.y = XM_PIDIV4 * random;
	t.scale = m_initTransform.scale;
	SetTransform(t);	

	SetActive(false);
}

void Plant::Initialize()
{
	m_timer = 0.0f;
	m_state = E_STATE::PLANT_READY_TO_SEED;


	float random = Random::Range(0.0f, 0.5f);
	m_finalTransform.scale = { random * 0.001f + 0.01f , random * 0.001f + 0.01f , random * 0.001f + 0.01f };
	m_finalTransform.position.y = random * -0.5f - 1.0f;

	Transform t = GetTransform();
	t.position.y = m_initTransform.position.y;
	t.rotation.y = XM_PIDIV4 * random;
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
		//if (m_timer >= m_lifeTime) m_state = E_STATE::PLANT_DEAD;
		return;
	}
	case E_STATE::PLANT_READY_TO_SEED:
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
	m_state = E_STATE::PLANT_READY_TO_SEED;

}

Field::Field() : 
	GameObject(),
	m_plant (nullptr),
	m_timer(0.0f),
	m_idleTime(Random::Range(3.0f, 100.0f)),
	m_state(E_STATE::FIELD_IDLE)
{
	GetRenderer()->SetMesh("plane");

	MeshDesc* pDesc = GetRenderer()->meshDesc();

	GetCollider()->SetBoundFromMesh(pDesc);
	SetTag("Field");
}

void Field::Update(float dt)
{
	if (m_timer < m_idleTime) m_timer += dt;

	UpdateState();
}

void Field::UpdateState()
{
	switch (m_state)
	{
	case E_STATE::FIELD_IDLE:
	{	
		if (m_timer >= m_idleTime) m_state = E_STATE::FIELD_READY_TO_AUTO_GROW;
		return;
	}
	case E_STATE::FIELD_READY_TO_AUTO_GROW:
	{
		// idle time�� ���� �ڵ����� �Ĺ��� �ɴ´�
		if (m_plant == nullptr)
		{
			m_plant = reinterpret_cast<Plant*>(DirectXGame::GetCurrentScene()->Instantiate<Plant>(
				"plant",
				Transform{},
				false,
				E_RenderLayer::Opaque));
			m_plant->SetParent(this);
		}

		if (m_plant->GetCurrentState() == Plant::E_STATE::PLANT_READY_TO_SEED && !m_plant->IsActive())
			m_plant->Seed();

		m_state = E_STATE::FIELD_OCCUPIED;
		return;
	}
	case E_STATE::FIELD_READY_TO_IDLE:
	{
		m_timer = 0.0f;
		m_state = E_STATE::FIELD_IDLE;

		return;
	}
	default: return;
	}
}

void Field::Interact()
{
	switch (m_state)
	{
	case E_STATE::FIELD_OCCUPIED:
	{
		// occupied�� �� ���콺 �Է��� ������, �� �ڶ� �Ĺ��� ��Ȯ�Ѵ�

		assert(m_plant != nullptr); 	// OCCUPIED ������ �� �ݵ�� plant�� �־�� ��
		auto plantState = m_plant->GetCurrentState();
		
		if ((plantState == Plant::E_STATE::PLANT_READY_TO_HARVEST) ||
			(plantState == Plant::E_STATE::PLANT_DEAD))
		{
			// �ڽ��� observer���� event Notify
			if (plantState == Plant::E_STATE::PLANT_READY_TO_HARVEST)
				Notify(reinterpret_cast<Object*>(m_plant), E_Event::FIELD_INTERACT_PLANT_HARVEST);

			m_plant->Harvest();
		}

		m_state = E_STATE::FIELD_READY_TO_IDLE;
		return;
	}
	case E_STATE::FIELD_IDLE:
	{
		// idle�� �� ���콺 �Է��� ������, �Ĺ��� �ɴ´�
		// ó������ �Ĺ��� ���� ��, instantiate �Ѵ�	
		if (m_plant == nullptr)
		{		
			m_plant = reinterpret_cast<Plant*>(DirectXGame::GetCurrentScene()->Instantiate<Plant>(
				"plant",
				Transform{},
				false,
				E_RenderLayer::Opaque));
			m_plant->SetParent(this);
		}

		if (m_plant->GetCurrentState() == Plant::E_STATE::PLANT_READY_TO_SEED && !m_plant->IsActive())
			m_plant->Seed();

		m_state = E_STATE::FIELD_OCCUPIED;
		return;
	}
	default: return;
	}
}

Tree::Tree() : GameObject()
{
	GetRenderer()->SetMesh("Assets/tree.fbx");
	GetRenderer()->SetMaterial("tree-mat");
	GetCollider()->SetBoundFromMesh(GetRenderer()->meshDesc());

	// change bound value manually
	auto bound = GetCollider()->GetBoundBox();
	XMFLOAT3 extent = bound.Extents;
	extent.x *= 2.0f;
	extent.z *= 2.0f;
	bound.Extents = extent;

	GetCollider()->SetBound(bound);
	
}

void Tree::Update(float dt)
{}