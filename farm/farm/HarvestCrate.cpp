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

	XMFLOAT3 localPivot = { -1.8f, 0.0f, -3.8f };

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

void HarvestCrate::Start()
{}

void HarvestCrate::Update(float dt)
{
	if (m_changed) {
		
		for (int i = 0; i < m_size; ++i)
		{
			//m_crops[i]->SetActive(i < m_count);
			m_crops[i]->SetActive(true);
		}

		m_changed = false;
	}
}

void HarvestCrate::Increase(int value)
{
	m_count += value;
	m_changed = true;
}

void HarvestCrate::Move(Transform transform)
{
	Transform t;
	t.position = transform.position;
	t.position.y = 0.0f;

	t.position.x += Random::Range(0.5f, 1.0f) * (Random::Boolean() ? 1.0f : -1.0f);
	t.position.z += Random::Range(0.5f, 1.0f) * (Random::Boolean() ? 1.0f : -1.0f);

	t.rotation = XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	t.rotation.y = Random::Range(0.0f, 35.0f) * XM_PI / 36.0f;
	
	SetTransform(t);
}