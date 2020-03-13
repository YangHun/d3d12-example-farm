#include "stdafx.h"
#include "Objects.h"
#include "Physics.h"
#include "DirectXGame.h"

Player::Player()
{

}

void Player::Start()
{

}

void Player::Update(float dt)
{
	if (GetAsyncKeyState(MK_LBUTTON) & 0x0001)
	{
		GetCursorPos(&m_cursorPos);

		// todo: Field GameObject ±¸ºÐÇÏ±â (¾¾¾Ñ ½É±â)
		GameObject* result = Physics::Raycast(DirectXGame::GetCurrentScene()->m_camera, m_cursorPos.x, m_cursorPos.y);

		DirectXGame::GetCurrentScene()->m_camera.picked = result;

		if (result != nullptr)
		{
			if (result->m_tag == "Field")
			{
				reinterpret_cast<Field*>(result)->Interact();
			}
		}
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
	m_cursorPos.x = 0;
	m_cursorPos.y = 0;
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
