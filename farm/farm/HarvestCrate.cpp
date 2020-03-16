#include "stdafx.h"
#include "Objects.h"
#include "DirectXGame.h"

HarvestCrate::HarvestCrate() :
	m_count(0),
	m_size(20),
	m_changed(true)
{
	GetRenderer()->SetMesh("Assets/cratebox.fbx");
	GetCollider()->SetBoundFromMesh(GetRenderer()->meshDesc());

	

	for (int i = 0; i < m_size; ++i) {
		auto obj = DirectXGame::GetCurrentScene()->Instantiate<GameObject>(
			"crops",
			Transform{
				//{localPivot.x + (i % 2) * 0.5f -1.0f, localPivot.y + (i / 4) * 0.2f + 0.2f, localPivot.z + ((i+1) % 2) * 0.5f },
				{localPivot.x + 0.5f + (-1.0f * (i % 5) * 0.25f), 0.7f + (i / 5) * 0.25f, localPivot.z - 0.6f},
				{XM_PI / 2.0f, Random::Range(-2.0f, 2.0f) * XM_PI / 36.0f , 0.0f},
				{0.0075f, 0.0075f, 0.0075f}
			},
			false);
		obj->SetParent(this);
		obj->GetRenderer()->SetMesh("Assets/carrot.fbx");
		m_crops.push_back(obj);
	}
}

void HarvestCrate::OnNotify(Object* object, E_Event event)
{
	switch (event)
	{
	case E_Event::FIELD_INTERACT_PLANT_HARVEST:
	{
		if (m_count < m_size) {
			Increase(1);
			MoveNearObject(reinterpret_cast<GameObject*>(object)->GetTransform());
		}

		return;
	}
	}
}

void HarvestCrate::Start()
{}

void HarvestCrate::Update(float dt)
{
	if (m_changed) {
		
		for (int i = 0; i < m_size; ++i)
		{
			m_crops[i]->SetActive(i < m_count);
		}

		m_changed = false;
	}
}

void HarvestCrate::Increase(int value)
{
	m_count += value;
	m_changed = true;
}

void HarvestCrate::MoveNearObject(Transform transform)
{
	XMFLOAT3 right = DirectXGame::GetCurrentScene()->GetCamera()->GetRightVector();

	Transform t;
	t.position = transform.position;
	t.position.x -= localPivot.x;
	t.position.y = 0.0f;
	t.position.z -= localPivot.z;

	t.position.x += 0.2f * (Random::Boolean() ? 1.0f : -1.0f) - right.x * 2;
	t.position.z += 0.2f * (Random::Boolean() ? 1.0f : -1.0f) - right.z * 2;

	t.rotation = XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	t.rotation.y = XM_PI / 36.0f * (Random::Boolean() ? 1.0f : -1.0f);
	
	SetTransform(t);
}