#pragma once
#include "DirectXApp.h"
#include "FrameResource.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ComPtr을 사용하기 위해, MeshDesc를 클래스로 바꾸는 등의 작업이 필요하다. todo.

struct MeshDesc {
	D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Mesh* mesh = nullptr;
	UINT indexCount;
	UINT startIndexLocation = 0;
	int baseVertexLocation = 0;

	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	ComPtr<ID3D12Resource> vertexUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> indexUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

class Scene
{
public:
	Scene(UINT id);

	void Initialize();

private:
	void BuildObject();

public: 
	UINT m_id;
	std::vector<MeshDesc*> m_objects;
	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	// current scene resources.
	
};

class DirectXGame
{

public:
	DirectXGame();

	void Initialize();

	UINT SceneCount() const { return m_allScenes.size(); }
	Scene* GetScene(UINT index) const { return m_allScenes[index].get(); }
	Scene* GetCurrentScene() const { return m_currentScene; }
	bool IsSceneChanged() const { return m_dirtyScene; }
	void SetUpdated() { m_dirtyScene = true; }

	std::unordered_map<std::string, std::unique_ptr<MeshDesc>> m_meshes;

private:
	void LoadAssets();
	void BuildScenes();
	void BuildSceneRenderObjects(Scene* scene);

	// resources.
	std::unordered_map<std::string, Mesh> m_models;

	Scene* m_currentScene = nullptr;
	std::vector<std::unique_ptr<Scene>> m_allScenes;

	bool m_dirtyScene = true;
};
