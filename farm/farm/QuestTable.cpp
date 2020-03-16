#include "stdafx.h"
#include "Objects.h"

QuestTable::QuestTable()
{
	SetTag("QuestTable");
	GetRenderer()->SetMesh("Assets/table.fbx");
	GetCollider()->SetBoundFromMesh(GetRenderer()->meshDesc());
}

void QuestTable::Start()
{

}

void QuestTable::Update(float dt)
{

}

void QuestTable::Interact()
{
	Notify(nullptr, E_Event::TABLE_INTERACT_CLICKED);
}