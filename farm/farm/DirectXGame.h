#pragma once
#include "DirectXApp.h"
#include "FrameResource.h"
#include "Camera.h"
#include "UploadBuffer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class Scene
{
public:
	Scene(UINT id);

	void Initialize();

	void UpdateObjectConstantBuffers();
private:
	void BuildObject();

public: 
	UINT m_id;
	std::vector<MeshDesc*> m_objects;
	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	// current scene resources.
	std::unique_ptr <UploadBuffer<ObjectConstantBuffer>> m_objConstantBuffers;

	//ComPtr<ID3D12Resource> m_cbUploadBuffer; // constant buffer uploader
	//BYTE* m_cbUploadData;
	//ObjectConstantBuffer* m_objConstantBuffer;


	Camera m_camera;
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
