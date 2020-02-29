#include "stdafx.h"
#include "DirectXGame.h"
#include "FbxLoader.h"
#include "Scenes.h"

DirectXGame::DirectXGame() 
{
	m_allScenes = std::vector<std::unique_ptr<Scene>>();
}

void DirectXGame::Initialize()
{
	LoadAssets();
	BuildScenes();
}

void DirectXGame::LoadAssets()
{
	// Game에서 쓸 모든 resource (fbx model, textures, ..) 를 load 한다

	std::vector<std::string> fbxName = {
		"Assets/deer.fbx",
		"Assets/tree.fbx"
	};

	FbxLoader loader = FbxLoader();

	for (auto a : fbxName)
	{
		if (m_models.find(a) == m_models.end()) {
			Mesh mesh;
			loader.Load(a.c_str(), &mesh);
			m_models[a] = mesh;

			auto meshDesc = std::make_unique<MeshDesc>();
			meshDesc->mesh = &m_models[a];
			meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

			m_meshes[a] = std::move(meshDesc);
		}
	}

	// create default triangle

	{
		Mesh mesh;
		mesh.vertices = {
			Vertex {{ 0.0f, 0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			Vertex {{0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			Vertex {{ -0.25f, -0.25f , 0.0f }, { 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}
		};

		mesh.indices = {
			0, 1, 2
		};

		m_models["triangle"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["triangle"];
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["triangle"] = std::move(meshDesc);
	}
}

void DirectXGame::BuildScenes()
{
	for (UINT i = 0; i < Assets::numScenes; ++i)
	{
		UINT _id = i;
		auto s = std::make_unique<Scene>(_id);

		s->Initialize();
		BuildSceneRenderObjects(s.get());

		m_allScenes.push_back(std::move(s));
	}

	m_currentScene = m_allScenes[0].get();
}

// Scene에 그려질 RenderObject 목록을 정의한다.
void DirectXGame::BuildSceneRenderObjects(Scene* scene)
{
	std::vector<MeshDesc*> objs = {
		m_meshes["triangle"].get(),
		m_meshes["triangle"].get(),
		m_meshes["triangle"].get(),

		m_meshes["Assets/deer.fbx"].get(),
		//m_meshes["Assets/tree.fbx"].get(),
	};

	std::vector<Transform> transform = {
		Transform {{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0, 0.0f}, {2.0f, 2.0f, 2.0f}},
		Transform {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0, 0.0f}, {2.0f, 2.0f, 2.0f}},
		Transform {{-2.0f, 0.0f, 0.0f}, {0.0f, 0.0, 0.0f}, {1.0f, 1.0f, 1.0f}},
		Transform { {0.0f, -1.0f, 0.0f}, {0.0f, XM_PI * 0.3f, 0.0f}, {0.002f, 0.002f, 0.002f} },
		//Transform { {0.0f, 0.0f, 0.0f}, { XM_PI * 0.3f, 0.0f, 0.0f}, {0.005f, 0.005f, 0.005f} },
	};

	UINT cbIndex = 0;
	for (auto obj : objs)
	{
		obj->transform = transform[cbIndex];
		obj->constantBufferId = cbIndex;
		scene->m_objects.push_back(obj);
	
		++cbIndex;
	}
}

Scene::Scene(UINT id) :
	m_id(id)
{
	m_camera.SetPosition(XMFLOAT3(0.0f, 0.0f, -3.0f));
	m_camera.SetFrustum(XM_PI * 0.25f);
}

void Scene::Initialize()
{
	// init scene here.
	BuildObject();
}

void Scene::BuildObject()
{
	// create game objects.
}

void Scene::UpdateObjectConstantBuffers()
{
	//UINT bufferSize = (sizeof(ObjectConstantBuffer));
	UINT bufferSize = 256;

	for (auto obj : m_objects)
	{
		if (obj->dirty)
		{
			auto t = obj->transform;
			
			XMMATRIX world = XMMatrixTranspose(XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z)
				* XMMatrixRotationRollPitchYaw(t.rotation.x, t.rotation.y, t.rotation.z)
				* XMMatrixTranslation(t.position.x, t.position.y, t.position.z));

			ObjectConstantBuffer objConstants;
			XMStoreFloat4x4(&objConstants.model, world);
			
			//memcpy(&m_objConstantBuffer + obj->constantBufferId * bufferSize, &objConstants, sizeof(objConstants));
			m_objConstantBuffers.get()->CopyData(obj->constantBufferId, objConstants);

			obj->dirty = false;
		}
	}
}

