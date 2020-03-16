#include "stdafx.h"
#include "Objects.h"

QuestInfo::QuestInfo()
{
	m_pBackground = std::make_unique<SpriteRenderer>(this);
	m_pBackground->SetSprite("default-black");
	m_pBackground->SetAlpha(0.5f);
	m_pBackground->SetRect(Rect{ 0.0f, 0.0f, 1280.0f, 720.0f });
	
	// content layout
	auto layout = GetSpriteRenderer();
	layout->SetSprite("scroll");
	Rect layoutRect = Rect{ 640.0f - 300.0f, 100.0f, 640.0f + 300.0f, 720.0f - 100.0f };
	layout->SetRect(layoutRect);

	auto text = GetUIText();
	text->SetTextDesc("black");
	text->SetRect(Rect{ layoutRect.left + 50.0f, layoutRect.top + 20.0f, layoutRect.right - 50.0f, layoutRect.bottom - 20.0f });
	text->SetText(L"당근 한 박스가 급하게 필요해.\n 품질은 상관 없고, 개수만 맞춰 주면 돼.\n갈아버릴 거거든.\n\n촉박한 만큼, 추가금을 얹지.\n가능하겠나?");

	SetActive(false);
}

void QuestInfo::Start()
{}

void QuestInfo::Update(float dt)
{}

void QuestInfo::Draw(ID2D1DeviceContext2* pDeviceContext)
{
	m_pBackground->Draw(pDeviceContext);
	GetSpriteRenderer()->Draw(pDeviceContext);
	GetUIText()->Draw(pDeviceContext);
}

void QuestInfo::OnNotify(Object* object, E_Event event)
{
	switch (event)
	{
	case E_Event::TABLE_INTERACT_CLICKED:
		SetActive(!IsActive());
		break;
	}
}