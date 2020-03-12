#include "stdafx.h"
#include "DirectXGame.h"
#include "FbxLoader.h"
#include "Physics.h"
#include "Component.h"
#include "Objects.h"

std::unordered_map<std::string, std::unique_ptr<Texture>> Assets::m_textures;
std::unordered_map<std::string, std::unique_ptr<MeshDesc>> Assets::m_meshes;
std::unordered_map<std::string, Mesh> Assets::m_models;
std::unordered_map<std::string, std::unique_ptr<Material>> Assets::m_materials;

Scene* DirectXGame::m_currentScene = nullptr;

DirectXGame::DirectXGame(CD3DX12_VIEWPORT* viewport) :
	m_pViewport(viewport)
{
	m_allScenes = std::vector<std::unique_ptr<Scene>>();
	Assets asset = Assets();	// call LoadAssets() in Assets class constructor.
}

void DirectXGame::Initialize()
{
	BuildScenes();
	m_player = std::make_unique<Player>();
}

void DirectXGame::BuildScenes()
{
	for (UINT i = 0; i < 1; ++i)
	{
		UINT _id = i;
		auto s = std::make_unique<Scene>(_id, m_pViewport);

		s->Initialize();
		BuildSceneRenderObjects(s.get());

		m_allScenes.push_back(std::move(s));
	}

	m_currentScene = m_allScenes[0].get();
}

// Scene에 그려질 RenderObject 목록을 정의한다.
void DirectXGame::BuildSceneRenderObjects(Scene* scene)
{

	// create fields
	{
		int activeNum = 5;

		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < 5; ++j)
			{
				scene->Instantiate<Field>(
					"Field_" + std::to_string(i * 10 + j), 
					Transform{
						{0.0f + (2.1f) * i, 0.0f, 0.0f + (2.1f) * j},
						{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} 
					},
					(i < activeNum) && (j < activeNum));
			}
		}
	}

	// a house.
	{
		auto obj = scene->Instantiate<GameObject>(
			"House",
			Transform{
				{-12.5f, 0.0f, 5.0f},
				{0.0f, XM_PI / 2.0f, 0.0f},
				{0.2f, 0.2f, 0.2f}
			},
			true);

		obj->m_renderer.SetMesh("Assets/house.fbx");
	}


	// a plant.
	{
		auto obj = scene->Instantiate<Plant>();
		obj->m_name = "Plant_1";
	}
}


void DirectXGame::OnKeyDown(UINT8 key)
{
	m_currentScene->m_camera.OnKeyDown(key);
}

void DirectXGame::OnKeyUp(UINT8 key)
{
	m_currentScene->m_camera.OnKeyUp(key);
}

bool _isCasting = false;

void DirectXGame::OnMouseDown(UINT8 btnState, int x, int y)
{
	
	if (!_isCasting)
	{
		_isCasting = true;
		GameObject* result = Physics::Raycast(m_currentScene->m_camera, x, y);

		m_currentScene->m_camera.picked = result;
		
		_isCasting = false;
	}
}

void DirectXGame::OnMouseUp(UINT8 btnState, int x, int y)
{

}

void DirectXGame::OnMouseMove(UINT8 btnState, int x, int y)
{
	m_currentScene->m_camera.OnMouseMove(btnState, x, y);
}

void DirectXGame::OnMouseLeave(UINT8 btnState, int x, int y)
{
	RECT window;
	GetClientRect(Win32Application::GetHwnd(), &window);
	
	POINT center;
	center.x = (window.left + window.right) / 2;
	center.y = (window.top + window.bottom) / 2;

	m_currentScene->m_camera.ResetMousePos(center);
}

Scene::Scene(UINT id, CD3DX12_VIEWPORT* viewport) :
	m_id(id),
	m_camera(viewport)
{
}

void Scene::Initialize()
{
	// init scene here.
	BuildObject();
}

template<typename T>
GameObject* Scene::Instantiate(std::string name, Transform transform, bool active)
{
	auto obj = Instantiate<T>();
	obj->m_name = name;
	obj->m_transform = transform;
	obj->m_active = active;

	return obj;
}

template<typename T>
GameObject* Scene::Instantiate()
{
	auto obj = std::make_unique<T>();

	int id = m_allObjects.size() + m_objWaitQueue.size();
	obj->m_bufferId = id;
	
	// T class의 constructor에서 초기화한 기본 값을 따른다.
	//obj->transform = Transform {}
	//obj->name = "new GameObject";
	obj->m_active = true;
	obj->SetDirty(true);

	m_objWaitQueue.push(std::move(obj));
	return m_objWaitQueue.back().get();
	
	/*m_renderObjects.push_back(obj.get());
	m_allObjects.push_back(std::move(obj));
	return m_allObjects.back().get();*/
}

void Scene::BuildObject()
{
	// create and initialize game objects.


	// init camera
	{
		m_camera.SetPosition(XMFLOAT3(-5.0f, 6.0f, -6.0f));
		m_camera.SetRotation(XMFLOAT3(30.0f, 45.0f, 0.0f));
		m_camera.SetFrustum(XM_PI * 0.25f);
	}
}

void Scene::Update(float dt)
{
	while (!m_objWaitQueue.empty())
	{
		auto front = std::move(m_objWaitQueue.front());
		m_objWaitQueue.pop();

		front->SetDirty(true);
		m_renderObjects.push_back(front.get());
		m_allObjects.push_back(std::move(front));
		
	}

	for (auto& i : m_allObjects)
	{
		auto obj = i.get();
		if (obj->m_active) obj->Update(dt);
	}

	m_camera.Update(dt);
	UpdateObjectConstantBuffers();
}

void Scene::UpdateObjectConstantBuffers()
{
	for (auto obj : m_renderObjects)
	{
		if (obj->IsDirty() && obj->m_active)
		{
			// GPU에서 사용할 것이므로 transpose 해준다. 
			// CPU 는 (column)(row), GPU는 (row)(column)
			XMMATRIX world = XMMatrixTranspose(obj->GetWorldMatrix());

			ObjectConstantBuffer objConstants;
			XMStoreFloat4x4(&objConstants.model, world);
			objConstants.matIndex = obj->m_renderer.GetMaterialIndex();

			//memcpy(&m_objConstantBuffer + obj->constantBufferId * bufferSize, &objConstants, sizeof(objConstants));
			m_objConstantBuffers.get()->CopyData(obj->m_bufferId, objConstants);

			obj->SetDirty(false);
		}
	}
}


Assets::Assets()
{
	// Create the default material and texture.
	{
		auto mat = std::make_unique<Material>();
		mat->name = "default-mat";
		mat->diffuseMapIndex = 0;
		mat->bufferId = 0;

		m_materials[mat->name] = std::move(mat);

		auto tex = std::make_unique<Texture>();
		tex->name = "default-white";
		tex->filePath = L"Textures/white.dds";
		
		m_textures[tex->name] = std::move(tex);
	}


	// Load meshes and textures.
	{
		std::vector<std::string> fbxName = {
			//"Assets/deer.fbx",
			"Assets/plant.fbx",
			"Assets/house.fbx",
		};

		FbxLoader loader = FbxLoader(&Assets::m_textures, &Assets::m_materials);

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

		mesh.minBound.x = -0.25f;
		mesh.maxBound.x = 0.25f;
		mesh.minBound.y = -0.25;
		mesh.maxBound.y = 0.5f;

		mesh.minBound.z = -0.1f;
		mesh.maxBound.z = 0.1f;

		m_models["triangle"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["triangle"];
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["triangle"] = std::move(meshDesc);
	}

	// create default plane
	{
		Mesh mesh;
		mesh.vertices = {
			Vertex {{ -1.0f, 0.0f, -1.0f }, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ -1.0f, 0.0f, 1.0f }, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ 1.0f, 0.0f, 1.0f }, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ 1.0f, 0.0f, -1.0f }, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		};

		mesh.indices = {
			0, 1, 2,
			0, 2, 3
		};

		mesh.minBound.x = -1.0f;
		mesh.maxBound.x = 1.0f;
		mesh.minBound.z = -1.0f;
		mesh.maxBound.z = 1.0f;

		mesh.minBound.y = -0.01f;
		mesh.maxBound.y = 0.01f;

		m_models["plane"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["plane"];
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["plane"] = std::move(meshDesc);
	}

}
