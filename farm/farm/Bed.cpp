#include "stdafx.h"
#include "Objects.h"

Bed::Bed() : GameObject()
{
	GetRenderer()->SetMesh("Assets/bed.fbx");
	GetCollider()->SetBoundFromMesh(GetRenderer()->meshDesc());
}

void Bed::Start()
{

}

void Bed::Update(float dt)
{

}

void Bed::Interact()
{

}