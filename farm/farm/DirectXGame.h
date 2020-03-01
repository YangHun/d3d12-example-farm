#pragma once
#ifndef DIRECTX_GAME_H_
#define DIRECTX_GAME_H_

#include "DirectXApp.h"
#include "FrameResource.h"
#include "Camera.h"
#include "UploadBuffer.h"
#include "Component.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class Scene
{
public:
	Scene(UINT id);

	void Initialize();

	void Update();
private:
	void BuildObject();
	void UpdateObjectConstantBuffers();

public: 
	UINT m_id;
	//std::vector<MeshDesc*> m_objects;

	std::vector<std::unique_ptr<Object>> m_allObjects;

	std::vector<Object*> m_renderObjects;

	// current scene resources.
	std::unique_ptr <UploadBuffer<ObjectConstantBuffer>> m_objConstantBuffers;


	Camera m_camera;
};

static class Assets {
public:
	Assets();
	static void LoadAssets();
	static std::unordered_map<std::string, std::unique_ptr<MeshDesc>> m_meshes;
	static std::unordered_map<std::string, Mesh> m_models;
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

private:
	void BuildScenes();
	void BuildSceneRenderObjects(Scene* scene);

	// resources.
	std::unordered_map<std::string, Mesh> m_models;

	Scene* m_currentScene = nullptr;
	std::vector<std::unique_ptr<Scene>> m_allScenes;

	bool m_dirtyScene = true;
};

#endif