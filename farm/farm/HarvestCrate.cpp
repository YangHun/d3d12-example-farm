#include "stdafx.h"
#include "Objects.h"
#include "DirectXGame.h"

HarvestCrate::HarvestCrate() :
	m_count(0),
	m_size(20),
	m_changed(true)
{
	GetRenderer()->SetMesh("Assets/cratebox.fbx");
	
	auto collider = GetCollider();
	collider->SetBoundFromMesh(GetRenderer()->meshDesc());	

	// Bound Àç¼³Á¤
	auto bound = collider->GetBoundBox();
	bound.Center.x = localPivot.x;
	bound.Center.z = localPivot.z;
	bound.Extents.z *= 0.4f;
	bound.Extents.x *= 0.75f;
	
	collider->SetBound(bound);

	for (int i = 0; i < m_size; ++i) {
		auto obj = DirectXGame::GetCurrentScene()->Instantiate<GameObject>(
			"crops",
			Transform{
				{localPivot.x + 0.5f + (-1.0f * (i % 5) * 0.25f), 0.7f + (i / 5) * 0.25f, localPivot.z - 0.6f},
				{XM_PI / 2.0f, Random::Range(-2.0f, 2.0f) * XM_PI / 36.0f , 0.0f},
				{0.0075f, 0.0075f, 0.0075f}
			},
			false,
			E_RenderLayer::Opaque);
		obj->SetParent(this);
		obj->GetRenderer()->SetMesh("Assets/carrot.fbx");
		obj->GetRenderer()->SetMaterial("carrot-mat");
		m_crops.push_back(obj);
	}
}


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

void HarvestCrate::MoveNearObject(GameObject* object)
{
	XMFLOAT3 right = DirectXGame::GetCurrentScene()->GetCamera()->GetRightVector();


	Transform t = GetTransform();
	Transform objTransform = object->GetTransform();

	XMMATRIX world = object->GetWorldMatrix();
	XMVECTOR pos = XMLoadFloat3(&objTransform.position);
	 
	XMStoreFloat3(&t.position, XMVector3Transform(pos, world));

	t.position.x -= localPivot.x;
	t.position.y = 0.0f;
	t.position.z -= localPivot.z;

	t.position.x += 0.2f * (Random::Boolean() ? 1.0f : -1.0f) - right.x * 2;
	t.position.z += 0.2f * (Random::Boolean() ? 1.0f : -1.0f) - right.z * 2;

	t.rotation = XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	t.rotation.y = XM_PI / 36.0f * (Random::Boolean() ? 1.0f : -1.0f);
	
	SetTransform(t);
}

CrateManager::CrateManager() :
	Object(),
	m_spawnCount(0),
	m_current(nullptr),
	m_spawnPoint(Transform{
			{0.0f, 0.0f, 0.0f},
			{0.0f, XM_PI / 36.0f, 0.0f},
			{1.0f, 1.0f, 1.0f}})
{
	for (size_t i = 0; i < 10; ++i)
	{
		auto obj = DirectXGame::GetCurrentScene()->Instantiate<HarvestCrate>(
			"Crate_" + std::to_string(i),
			Transform{},
			false,
			E_RenderLayer::Opaque);
		m_pool.push(reinterpret_cast<GameObject*>(obj));
	}

	m_current = Spawn(m_spawnPoint);
}

void CrateManager::Update(float dt)
{

}

void CrateManager::OnNotify(Object* object, E_Event event)
{
	switch (event)
	{
	case E_Event::FIELD_INTERACT_PLANT_HARVEST:
	{
		if (m_current == nullptr) return;
		
		if (m_current->IsFull())
		{
			Stack(m_current);
			m_current = Spawn(m_spawnPoint);
		}
		
		if (m_current == nullptr) return;
		m_current->Increase(1);
		m_current->MoveNearObject(reinterpret_cast<GameObject*>(object));

		return;
	}
	}
}

void CrateManager::Stack(HarvestCrate* target)
{
	int row = 5;

	Transform t = Transform{ 
		{-3.5f + m_spawnCount / row * 2.5f, 0.0f, 2.0f - m_spawnCount % row * 2.5f},
		{0.0f, Random::Range(-0.1f, 0.1f), 0.0f},
		{1.0f, 1.0f, 1.0f} };

	target->SetTransform(t);
}

HarvestCrate* CrateManager::Spawn(Transform transform)
{
	if (m_pool.empty()) return nullptr;
	
	auto front = m_pool.front();
	m_pool.pop();
	front->SetTransform(transform);
	front->SetActive(true);

	++m_spawnCount;

	return reinterpret_cast<HarvestCrate*>(front);
}
