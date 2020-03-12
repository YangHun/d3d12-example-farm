#include "stdafx.h"
#include "Objects.h"

Player::Player()
{

}

void Player::Start()
{

}

void Player::Update(float dt)
{

}

void Player::OnMouseMove(WPARAM state, int x, int y)
{
	// todo: 카메라가 가리키는 대상 레이캐스트로 인식
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
