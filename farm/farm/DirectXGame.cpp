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


Scene::Scene(UINT id) :
	m_id(id),
	m_vertexBuffer(nullptr),
	m_indexBuffer(nullptr),
	m_vertexUploadBuffer(nullptr),
	m_indexUploadBuffer(nullptr)
{
}

void Scene::Initialize()
{
	// init scene here.
	LoadAssets();
}

void Scene::LoadAssets()
{
	
	for (int i = 0; i < 3; ++i)
	{
		float delta = (float) (i + 1) / 3.0f;
		m_vertice.push_back(Vertex({ { 0.0f + 0.3f * delta, 0.5f, 0.0f }, { 1.0f * delta, 0.0f, 0.0f, 1.0f } }));
		m_vertice.push_back(Vertex({ { 0.25f + 0.3f * delta, -0.5f, 0.0f }, { 0.0f, 1.0f * delta, 0.0f, 1.0f } }));
		m_vertice.push_back(Vertex({ { -0.25f + 0.3f * delta, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f * delta, 1.0f } }));

		for (int j = 0; j < 3; ++j) {
			m_indices.push_back((uint16_t)(i * 3 + j));
		}
	}

	// load assets
	//m_vertice.assign(Assets::triangleVertices[m_id].begin(), Assets::triangleVertices[m_id].end());
}

