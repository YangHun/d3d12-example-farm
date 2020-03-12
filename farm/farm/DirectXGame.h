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
	Scene(UINT id, CD3DX12_VIEWPORT* viewport);

	void Initialize();
	void Update(float dt);

	template<typename T>
	GameObject* Instantiate(std::string name, Transform transform, bool active);

private:
	void BuildObject();
	void UpdateObjectConstantBuffers();

public: 
	UINT m_id;
	//std::vector<MeshDesc*> m_objects;

	std::vector<std::unique_ptr<GameObject>> m_allObjects;

	std::vector<GameObject*> m_renderObjects;

	// current scene resources.
	std::unique_ptr <UploadBuffer<ObjectConstantBuffer>> m_objConstantBuffers;
	
	Camera m_camera;
};

static class Assets {
public:
	Assets();
	static std::unordered_map<std::string, std::unique_ptr<MeshDesc>> m_meshes;
	static std::unordered_map<std::string, Mesh> m_models;
	static std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	static std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
};

class DirectXGame
{

public:
	DirectXGame(CD3DX12_VIEWPORT* viewport);

	void Initialize();

	UINT SceneCount() const { return m_allScenes.size(); }
	Scene* GetScene(UINT index) const { return m_allScenes[index].get(); }
	static Scene* GetCurrentScene() { return m_currentScene; }
	bool IsSceneChanged() const { return m_dirtyScene; }
	void SetUpdated() { m_dirtyScene = true; }

	void OnKeyDown(UINT8 key);
	void OnKeyUp(UINT8 key);
	void OnMouseDown(UINT8 btnState, int x, int y);
	void OnMouseUp(UINT8 btnState, int x, int y);
	void OnMouseMove(UINT8 btnState, int x, int y);
	void OnMouseLeave(UINT8 btnState, int x, int y);

private:
	void BuildScenes();
	void BuildSceneRenderObjects(Scene* scene);

	// resources.
	std::unordered_map<std::string, Mesh> m_models;

	static Scene* m_currentScene;
	std::vector<std::unique_ptr<Scene>> m_allScenes;

	CD3DX12_VIEWPORT* m_pViewport;

	bool m_dirtyScene = true;
};

#endif