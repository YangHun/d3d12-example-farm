#include "stdafx.h"
#include "Objects.h"

Deer::Deer()
{
	m_renderer = MeshRenderer(this);
	m_renderer.SetMesh("Assets/deer.fbx");
}

void Deer::Start() {

}

void Deer::Update(float dt) {
	m_angle += 0.001f;
	m_transform.rotation.y = XM_PI * 0.3f + m_angle;
	m_dirty = true;
}
