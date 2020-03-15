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
	GameObject* Instantiate(std::string name, Transform transform, bool active)
	{
		auto obj = Instantiate<T>();
		obj->SetName(name);
		obj->SetActive(active);
		obj->SetTransform(transform);

		return obj;
	}

	template<typename T>
	GameObject* Instantiate()
	{
		auto obj = std::make_unique<T>();

		int id = m_allObjects.size() + m_objWaitQueue.size();
		obj->SetBufferId(id);
		obj->SetDirty();

		m_objWaitQueue.push(std::move(obj));
		return m_objWaitQueue.back().get();
	}

private:
	void BuildObject();
	void UpdateObjectConstantBuffers();

public: 
	UINT m_id;

	// �̹� �����ӿ� Instantiate �� ������Ʈ�� waiting queue�� �̸� ����ߴٰ� 
	// ���� Update()�� ȣ��Ǵ� �������� m_allObjects�� ����Ͽ�
	// m_allObjects�� ��ȸ�ϴ� ���߿��� ���� �ٲ��� �ʰ� �Ѵ�.
	std::queue<std::unique_ptr<GameObject>> m_objWaitQueue;

	// scene�� �����ϴ� ��� GameObject 
	std::vector<std::unique_ptr<GameObject>> m_allObjects;
	// Engine���� ������ �׸��� ������Ʈ
	std::vector<GameObject*> m_renderObjects;

	// object constant buffers for objects in current scene.
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
	void BuildSceneRenderObjects(Scene* scene);

	// resources.
	std::unordered_map<std::string, Mesh> m_models;

	static Scene* m_currentScene;
	static std::unique_ptr<Player> m_player;
	std::vector<std::unique_ptr<Scene>> m_allScenes;

	CD3DX12_VIEWPORT* m_pViewport;

	bool m_dirtyScene = true;
};

#endif