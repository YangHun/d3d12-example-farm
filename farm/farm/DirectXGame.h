#pragma once
#ifndef DIRECTX_GAME_H_
#define DIRECTX_GAME_H_

#include "DirectXApp.h"
#include "FrameResource.h"
#include "UploadBuffer.h"
#include "Camera.h"
#include "Component.h"

#include <queue>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class Scene
{
public:
	Scene(UINT id, CD3DX12_VIEWPORT* viewport);

	void Initialize();
	void Update(float dt);

	template<typename T>
	GameObject* Instantiate(E_RenderLayer layer)
	{
		auto obj = std::make_unique<T>();
		int layerID = static_cast<int>(layer);
		int id = -1;
		obj->SetLayer(layerID);
		obj->SetBufferId(id);
		obj->SetDirty();

		m_objQueue[layerID].push(std::move(obj));
		return m_objQueue[layerID].back().get();
	}

	template<typename T>
	GameObject* Instantiate(std::string name, Transform transform, bool active, E_RenderLayer layer)
	{
		auto obj = Instantiate<T>(layer);
		obj->SetName(name);
		obj->SetActive(active);
		obj->SetTransform(transform);

		return obj;
	}

	template<typename T>
	UIObject* Instantiate(std::string name)
	{
		auto obj = std::make_unique<T>();

		int id = -1;
		obj->SetBufferId(id);
		obj->SetDirty();

		m_uiQueue.push(std::move(obj));
		return m_uiQueue.back().get();
	}

	std::vector<UIObject*> GetAllUIObjects();
	std::vector<GameObject*> GetObjectsByLayer(E_RenderLayer layer)
	{
		return m_layeredObjects[static_cast<int>(layer)];
	}

	Camera* GetCamera() { return &m_camera; }

private:
	void BuildObject();
	void UpdateInstanceData();
	void AssignInstantiatedObjects();

public: 
	// object constant buffers for objects in current scene.
	std::unique_ptr <UploadBuffer<ObjectConstantBuffer>> m_objConstantBuffers;

private:
	UINT m_id;

	// 이번 프레임에 Instantiate 된 오브젝트는 waiting queue에 미리 등록했다가 
	// 다음 프레임의 Update()가 호출되는 시점에서 m_allObjects에 등록하여
	// m_allObjects를 순회하는 도중에는 값이 바뀌지 않게 한다.
	std::unordered_map<int, std::queue<std::unique_ptr<GameObject>>> m_objQueue;
	std::queue<std::unique_ptr<UIObject>> m_uiQueue;

	// scene에 존재하는 모든 Game objects
	std::vector<std::unique_ptr<GameObject>> m_allObjects;
	std::unordered_map <int, std::vector<GameObject*>> m_layeredObjects;

	// scene에 존재하는 모든 UI Objects
	std::vector<std::unique_ptr<UIObject>> m_allUIs;

	Camera m_camera;
};

static class Assets {
public:
	Assets();
	static std::unordered_map<std::string, std::unique_ptr<MeshDesc>> m_meshes;
	static std::unordered_map<std::string, Mesh> m_models;
	static std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	static std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
	static std::unordered_map<std::string, std::unique_ptr<Sprite>> m_sprites;
	static std::unordered_map<std::string, std::unique_ptr<TextDesc>> m_texts;

	static std::vector<Texture*> GetOrderedTextures();
	static std::vector<MeshDesc*> GetMeshDesc();
	static std::vector<Sprite*> GetSprites();
	static std::vector<TextDesc*> GetTexts();
	static Material* GetMaterialByID(UINT id);

};

class DirectXGame
{
public:
	DirectXGame(CD3DX12_VIEWPORT* viewport);

	void Initialize();

	static Scene* GetCurrentScene() { return m_currentScene; }
	static Player* GetPlayer() { return m_player.get(); }
	UINT SceneCount() const { return m_allScenes.size(); }
	Scene* GetScene(UINT index) const { return m_allScenes[index].get(); }
	
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

	// resources.
	std::unordered_map<std::string, Mesh> m_models;

	static Scene* m_currentScene;
	static std::unique_ptr<Player> m_player;
	std::vector<std::unique_ptr<Scene>> m_allScenes;

	CD3DX12_VIEWPORT* m_pViewport;

	bool m_dirtyScene = true;
};

inline void BuildSceneObjects(Scene* scene);

#endif