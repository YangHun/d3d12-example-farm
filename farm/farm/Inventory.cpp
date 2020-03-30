#include "stdafx.h"
#include "Objects.h"
#include "DirectXGame.h"

InventorySlot::InventorySlot()
{
	auto renderer = GetSpriteRenderer();
	renderer->SetSprite("white");
}

void InventorySlot::Update(float dt)
{

}

Inventory::Inventory()
{
	auto scene = DirectXGame::GetCurrentScene();

	int width = 1280;
	int height = 720;

	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			auto slot = scene->Instantiate<InventorySlot>("slot_" + std::to_string(i) + std::to_string(j));

			XMFLOAT2 pos = {
				width / 4.0f + i * (width / 20.0f) + 5.0f,
				height / 2.0f + j * (height / 10.0f)
			};
			float size = 50.0f;

			slot->GetSpriteRenderer()->SetRect(Rect{ pos.x - size/2.0f, pos.y - size/2.0f, pos.x + size/2.0f, pos.y + size/2.0f });
			slot->SetParent(this);

			m_slots.push_back(reinterpret_cast<InventorySlot*>(slot));
		}
	}

	SetActive(false);
}

void Inventory::Update(float dt)
{

}

void Inventory::Draw(ID2D1DeviceContext2* pDeviceContext)
{
	for (auto& obj : m_slots)
	{
		obj->Draw(pDeviceContext);
	}
}

void Inventory::OnNotify(Object* object, E_Event event)
{
	
}