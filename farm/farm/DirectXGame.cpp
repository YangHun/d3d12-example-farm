#include "stdafx.h"
#include "D3DGameEngine.h"
#include "Scenes.h"

DirectXGame::DirectXGame() 
{
	m_allScenes = std::vector<std::unique_ptr<Scene>>();
}

void DirectXGame::Initialize()
{
	BuildScenes();
}

void DirectXGame::BuildScenes()
{
	for (UINT i = 0; i < Assets::numScenes; ++i)
	{
		UINT _id = i;
		auto s = std::make_unique<Scene>(_id);

		s->Initialize();

		m_allScenes.push_back(std::move(s));
	}

	m_currentScene = m_allScenes[0].get();
}

void DirectXGame::GetCurrentSceneBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, D3D12_INDEX_BUFFER_VIEW& indexBufferView)
{
	if (!m_currentScene) return;

	vertexBufferView = m_currentScene->m_vertexBufferView;
	indexBufferView = m_currentScene->m_indexBufferView;
}



DirectXGame::Scene::Scene(UINT id) :
	m_id(id),
	m_vertexBuffer(nullptr),
	m_indexBuffer(nullptr),
	m_vertexUploadBuffer(nullptr),
	m_indexUploadBuffer(nullptr)
{
	m_data = std::vector<Vertex>();
}

void DirectXGame::Scene::Initialize()
{
	// init scene here.
	LoadAssets();
}

void DirectXGame::Scene::LoadAssets()
{
	// load assets
	m_data.assign(Assets::triangleVertices[m_id].begin(), Assets::triangleVertices[m_id].end());
}
