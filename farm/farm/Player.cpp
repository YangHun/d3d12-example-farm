#include "stdafx.h"
#include "Objects.h"
#include "Physics.h"
#include "DirectXGame.h"

inline void Interact(Player* player, GameObject* object);

Player::Player()
{

}

void Player::Update(float dt)
{
	
	Transform t = GetTransform();
	t.position = DirectXGame::GetCurrentScene()->GetCamera()->GetEyePosition();
	SetTransform(t);

	// windows message queue는 queue에 등록된 순서대로 이벤트를 처리하기 때문에
	// 실제 입력된 시각에 비해 미세한 딜레이가 생길 수 있다.
	// 딜레이를 피하기 위해, 마우스 입력은 async하게 감지하여 처리한다.
	if (GetAsyncKeyState(MK_LBUTTON) & 0x0001)
	{
		/*GetCursorPos(&m_cursorPos);
		ScreenToClient(Win32Application::GetHwnd(), &m_cursorPos);
		GameObject* result = Physics::Raycast(DirectXGame::GetCurrentScene()->m_camera, m_cursorPos.x, m_cursorPos.y);*/

		Camera* cam = DirectXGame::GetCurrentScene()->GetCamera();
		GameObject* result = Physics::Raycast(cam, 640, 360);
		

		cam->picked = result;

		if (result != nullptr)
		{
			Interact(this, result);			
		}
	}
}

void Interact(Player* player, GameObject* object)
{
	std::string tag = object->GetTag();

	if (tag == "Field")
	{
		reinterpret_cast<Field*>(object)->Interact();
	}
	else if (tag == "QuestTable")
	{
		reinterpret_cast<QuestTable*>(object)->Interact();
	}
}

std::wstring Player::PrintPlayerInfo()
{
	std::wostringstream oss;
	oss << "-------player info----------\n";
	oss << "# crops: " << m_crops << "\n";
	oss << "# coins: " << m_coins  << "\n";

	return oss.str();
}

void Player::OnNotify(Object* object, E_Event event)
{
	switch (event)
	{
	case E_Event::FIELD_INTERACT_PLANT_HARVEST:
	{
		m_crops++;
		return;
	}
	case E_Event::TABLE_INTERACT_CLICKED:
		if (m_crops > 0) 
		{
			m_crops--;
			m_coins += 100;
		}
		return;
	}
}

void Player::OnMouseDown(WPARAM state, int x, int y)
{
	/*if ((state & MK_LBUTTON) != 0) {
		m_leftMouseDown = true;
		m_leftMouseUp = false;
		m_cursorPos.x = x;
		m_cursorPos.y = y;
	}*/
}

void Player::OnMouseUp(WPARAM state, int x, int y)
{
	
}

void Player::OnMouseMove(WPARAM state, int x, int y)
{

}

void Player::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_pressedW = true;
		break;
	case 'A':
		m_pressedA = true;
		break;
	case 'S':
		m_pressedS = true;
		break;
	case 'D':
		m_pressedD = true;
		break;
	}
}

void Player::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_pressedW = false;
		break;
	case 'A':
		m_pressedA = false;
		break;
	case 'S':
		m_pressedS = false;
		break;
	case 'D':
		m_pressedD = false;
		break;
	}
}
