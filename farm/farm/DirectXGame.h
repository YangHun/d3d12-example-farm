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
	Object* Instantiate()
	{
		auto obj = std::make_unique<T>();
		obj->SetLayer(-1);
		obj->SetBufferId(-1);

		m_queue.push(std::move(obj));
		return reinterpret_cast<Object*>(m_queue.back().get());
	}

	template<typename T>
	GameObject* Instantiate(std::string name, Transform transform, bool active, E_RenderLayer layer)
	{
		auto obj = reinterpret_cast<GameObject*>(Instantiate<T>());
		obj->SetLayer(static_cast<int>(layer));
		obj->SetActive(active);
		obj->SetTransform(transform);
		obj->SetName(name);

		return obj;
	}

	template<typename T>
	UIObject* Instantiate(std::string name)
	{
		auto obj = reinterpret_cast<UIObject*>(Instantiate<T>());
		obj->SetLayer(static_cast<int>(E_RenderLayer::Count));
		obj->SetName(name);

		return obj;
	}

	std::vector<UIObject*> GetAllUIObjects() const { return m_UIObjects; };
	std::vector<GameObject*> GetObjectsByLayer(E_RenderLayer layer)
	{
		return m_GameObjects[static_cast<int>(layer)];
	}
	std::vector<GameObject*> GetAllGameObjects();

	Camera* GetCamera() { return &m_camera; }

private:
	void BuildObject();
	void UpdateInstanceData();
	void AssignInstantiatedObjects();

private:
	UINT m_id;

	// 이번 프레임에 Instantiate 된 오브젝트는 waiting queue에 미리 등록했다가 
	// 다음 프레임의 Update()가 호출되는 시점에서 m_allObjects에 등록하여
	// m_allObjects를 순회하는 도중에는 값이 바뀌지 않게 한다.
	std::queue<std::unique_ptr<Object>> m_queue;
	
	// scene에 존재하는 모든 Objects
	std::vector<std::unique_ptr<Object>> m_objects;

	std::unordered_map <int, std::vector<GameObject*>> m_GameObjects;
	std::vector<UIObject*> m_UIObjects;

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