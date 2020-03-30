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
	layout->SetSprite("paper");
	Rect layoutRect = Rect{ 640.0f - 200.0f, 100.0f, 640.0f + 200.0f, 720.0f - 100.0f };
	layout->SetRect(layoutRect);

	auto text = GetUIText();
	text->SetTextDesc("black-center");
	text->SetRect(Rect{ layoutRect.left + 50.0f, layoutRect.top + 20.0f, layoutRect.right - 50.0f, layoutRect.bottom - 20.0f });
	text->SetText(L"비록 내일\n\n세계의 종말이 올지라도\n\n나는 오늘\n\n한 박스의 당근을 심겠다");

	SetActive(false);
}

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