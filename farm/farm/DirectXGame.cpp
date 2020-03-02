#include "stdafx.h"
#include "DirectXGame.h"
#include "FbxLoader.h"

std::unordered_map<std::string, std::unique_ptr<MeshDesc>> Assets::m_meshes;
std::unordered_map<std::string, Mesh> Assets::m_models;

DirectXGame::DirectXGame() 
{
	m_allScenes = std::vector<std::unique_ptr<Scene>>();
	Assets asset = Assets();	// call LoadAssets() in Assets class constructor.
}

void DirectXGame::Initialize()
{
	BuildScenes();
}

void DirectXGame::BuildScenes()
{
	for (UINT i = 0; i < 1; ++i)
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

	// create fields
	{
		int activeNum = 3;

		for (size_t i = 0; i < 10; ++i)
		{
			for (size_t j = 0; j < 10; ++j)
			{
				auto obj = std::make_unique<Field>();
				obj->m_transform = Transform{ 
					{0.0f + (2.1f) * i, 0.0f, 0.0f + (2.1f) * j}, 
					{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };
				
				bool active = (i < activeNum) && (j < activeNum);
				int id = scene->m_allObjects.size();

				obj->m_active = active;			
				obj->m_bufferId = id;

				scene->m_allObjects.push_back(std::move(obj));
				if (active) scene->m_renderObjects.push_back(scene->m_allObjects[id].get());
			}
		}
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

void DirectXGame::OnMouseDown(UINT8 btnState, int x, int y)
{

}

void DirectXGame::OnMouseUp(UINT8 btnState, int x, int y)
{

}

void DirectXGame::OnMouseMove(UINT8 btnState, int x, int y)
{
	m_currentScene->m_camera.OnMouseMove(btnState, x, y);
}

Scene::Scene(UINT id) :
	m_id(id)
{
}

void Scene::Initialize()
{
	// init scene here.
	BuildObject();
}

void Scene::BuildObject()
{
	// create and initialize game objects.


	// init camera
	{
		m_camera.SetPosition(XMFLOAT3(-5.0f, 6.0f, -6.0f));
		m_camera.SetRotation(XMFLOAT3(30.0f, 45.0f, 0.0f));
		m_camera.SetFrustum(XM_PI * 0.25f);

		/*EventHandler handler = [](WPARAM param) {
			m_camera.OnKeyDown(param);
		}
		m_inputHandler.m_keyDown += EventHandler(&Camera::OnKeyDown);*/

		//std::function<void(WPARAM)> pf = std::bind(&IKeyHandler::OnKeyDown, &m_camera, std::placeholders::_1);
		//m_inputHandler.Assign(E_InputType::KEY_DOWN, pf);
		//m_inputHandler.Assign(E_InputType::KEY_UP, m_camera.OnKeyUp);
	}
}

void Scene::Update()
{
	m_camera.Update();
	for (auto& i : m_allObjects)
	{
		auto obj = i.get();
		if (obj->m_active) obj->Update();
	}

	UpdateObjectConstantBuffers();
}

void Scene::UpdateObjectConstantBuffers()
{
	for (auto obj : m_renderObjects)
	{
		if (obj->IsDirty())
		{
			auto t = obj->m_transform;
			
			XMMATRIX world = XMMatrixTranspose(XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z)
				* XMMatrixRotationRollPitchYaw(t.rotation.x, t.rotation.y, t.rotation.z)
				* XMMatrixTranslation(t.position.x, t.position.y, t.position.z));

			ObjectConstantBuffer objConstants;
			XMStoreFloat4x4(&objConstants.model, world);
			
			//memcpy(&m_objConstantBuffer + obj->constantBufferId * bufferSize, &objConstants, sizeof(objConstants));
			m_objConstantBuffers.get()->CopyData(obj->m_bufferId, objConstants);

			obj->SetDirty(false);
		}
	}
}


Assets::Assets()
{
	LoadAssets();
}

void Assets::LoadAssets()
{
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

	// create default plain
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

		m_models["plain"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["plain"];
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["plain"] = std::move(meshDesc);
	}

}