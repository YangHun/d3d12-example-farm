#include "stdafx.h"
#include "Objects.h"

Deer::Deer()
{
	GetRenderer()->SetMesh("Assets/deer.fbx");
}

void Deer::Start() {

}

void Deer::Update(float dt) {
	m_angle += 0.001f;

	Transform t = GetTransform();
	t.rotation.y = XM_PI * 0.3f + m_angle;
	SetTransform(t);

	SetDirty();
}
