#include "stdafx.h"
#include "DirectXGame.h"
#include "FbxLoader.h"
#include "Physics.h"
#include "Component.h"


std::unordered_map<std::string, std::unique_ptr<Texture>> Assets::m_textures;
std::unordered_map<std::string, std::unique_ptr<MeshDesc>> Assets::m_meshes;
std::unordered_map<std::string, Mesh> Assets::m_models;
std::unordered_map<std::string, std::unique_ptr<Material>> Assets::m_materials;
std::unordered_map<std::string, std::unique_ptr<Sprite>> Assets::m_sprites;
std::unordered_map<std::string, std::unique_ptr<TextDesc>> Assets::m_texts;

Scene* DirectXGame::m_currentScene = nullptr;
std::unique_ptr<Player> DirectXGame::m_player = nullptr;


DirectXGame::DirectXGame(CD3DX12_VIEWPORT* viewport) :
	m_pViewport(viewport)
{
	m_allScenes = std::vector<std::unique_ptr<Scene>>();
	Assets asset = Assets();	// call LoadAssets() in Assets class constructor.
}

void DirectXGame::Initialize()
{
	m_player = std::make_unique<Player>();
	BuildScenes();
}

void DirectXGame::BuildScenes()
{
	for (UINT i = 0; i < 1; ++i)
	{
		UINT _id = i;
		auto s = std::make_unique<Scene>(_id, m_pViewport);
		m_currentScene = s.get();

		s->Initialize();
		BuildSceneObjects(s.get());

		m_allScenes.push_back(std::move(s));
	}

	m_currentScene = m_allScenes[0].get();
}

// Scene에 그려질 RenderObject 목록을 정의한다.
void BuildSceneObjects(Scene* scene)
{
	// sky.
	{
		auto obj = scene->Instantiate<GameObject>(
			"sky",
			Transform{
				{0.0f, 0.0f, 0.0f},
				{0.0f, 0.0f, 0.0f},
				{1000.0f, 1000.0f, 1000.0f}
			},
			true,
			E_RenderLayer::Sky);
		auto renderer = obj->GetRenderer();
		renderer->SetMesh("sphere");
		renderer->SetMaterial("sky");

	}

	// quad for debug shadow.
	{
		auto obj = scene->Instantiate<GameObject>(
			"quad",
			Transform{
				{0.0f, 0.0f, 0.0f},
				{0.0f, 0.0f, 0.0f},
				{0.5f, 0.5f, 1.0f}
			},
			true,
			E_RenderLayer::Debug);

		obj->GetRenderer()->SetMesh("quad");
		obj->GetRenderer()->SetMaterial("default-mat");
	}

	// a table and a note.
	{
		auto obj = reinterpret_cast<QuestTable*>(scene->Instantiate<QuestTable>(
			"Table",
			Transform{
				{-8.0f, 1.1f, 1.6f},
				{0.0f, 0.0f, 0.0f},
				{0.03f, 0.02f, 0.03f}
			},
			true,
			E_RenderLayer::Opaque));

		auto ui = reinterpret_cast<QuestInfo*>(scene->Instantiate<QuestInfo>("quest-info"));
		obj->AddObserver(ui);
		obj->AddObserver(scene->GetCamera());
		obj->AddObserver(DirectXGame::GetPlayer());
	
		auto note = scene->Instantiate<GameObject>(
			"note",
			Transform{ {-8.3f, 2.5f, 2.2f}, {0.0f, 0.3f, 0.0f}, {0.02f, 0.02f, 0.02f} },
			true,
			E_RenderLayer::Opaque);
		note->GetRenderer()->SetMesh("Assets/Pen and Notepad.fbx");
		note->GetRenderer()->SetMaterial("note-mat");
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
			true,
			E_RenderLayer::Opaque);

		obj->GetRenderer()->SetMesh("Assets/house.fbx");
	}


	// plane to test shadow map.
	{
		auto obj = scene->Instantiate<GameObject>(
			"plane",
			Transform{
				{0.0f, -0.05f, 0.0f},
				{0.0f, 0.0f, 0.0f},
				{50.0f, 1.0f, 50.0f}
			},
			true,
			E_RenderLayer::Opaque);

		auto renderer = obj->GetRenderer();
		renderer->SetMesh("plane");
		
		Material mat = {
			0,
			"grass",
			Assets::m_textures["grass_tile"]->id
		};
		XMStoreFloat4x4(&mat.textureTile, XMMatrixScaling(10.0, 10.0f, 1.0f));
		
		renderer->SetMaterial(mat);
	}
	
	// center aim.
	{
		auto obj = scene->Instantiate<UIObject>("cross-aim");
		auto renderer = obj->GetSpriteRenderer();
		renderer->SetSprite("cross-aim");
		// 1280 x 720
		float size = 8;
		renderer->SetRect(Rect{ 640 - size,  360 - size, 640 + size , 360 + size });
	}

	

	// a bed and a pillow.
	{
		auto bed = scene->Instantiate<Bed>(
			"Bed",
			Transform{
				{-17.5f, 1.0f, 3.0f},
				{0.0f, 0.0f, 0.0f},
				{0.02f, 0.02f, 0.02f}
			},
			true,
			E_RenderLayer::Opaque);

		Transform pillowTransform = bed->GetTransform();
		pillowTransform.position.y += 1.3f;
		pillowTransform.position.z += -1.5f;
		pillowTransform.rotation.y += XM_PI / 10.0f;

		auto pillow = scene->Instantiate<GameObject>(
			"Pillow",
			pillowTransform,
			true,
			E_RenderLayer::Opaque);

		pillow->GetRenderer()->SetMesh("Assets/pillow.fbx");
	}
	
	// harvesting crate.
	auto crate = reinterpret_cast<HarvestCrate*>(scene->Instantiate<HarvestCrate>(
		"CrateBox",
		Transform{
			{0.0f, 0.0f, 0.0f},
			{0.0f, XM_PI / 36.0f, 0.0f},
			{1.0f, 1.0f, 1.0f}
		},
		true,
		E_RenderLayer::Opaque));
	

	// create fields
	{
		int activeNum = 50;

		for (int i = 0; i < 18; ++i)
		{
			for (int j = -18; j < 18; ++j)
			{
				auto obj = reinterpret_cast<Field*>(scene->Instantiate<Field>(
					"Field_" + std::to_string(i * 10 + j),
					Transform{
						{0.0f + (2.0f) * i, 0.0f, 0.0f + (2.0f) * j},
						{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
					},
					(i != 0 && j != 0),
					E_RenderLayer::Opaque));
				obj->AddObserver(DirectXGame::GetPlayer());
				obj->AddObserver(crate);
				obj->GetRenderer()->SetMaterial("field-mat");
			}
		}
	}

	// create trees (field side)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = -22; j < 22; ++j)
			{
				if ((i + j) % 3 != 0) continue;

				float scale = Random::Range(1.0f, 1.5f);

				scene->Instantiate<Tree>(
					"Tree_" + std::to_string(i * 10 + j),
					Transform{
						{40.0f + (2.0f) * i, 2.0f, 0.0f + (2.0f) * j},
						{0.0f, 0.0f, 0.0f}, {0.01f * scale, 0.01f * scale, 0.01f * scale}
					},
					true,
					E_RenderLayer::Opaque);
			}
		}

		for (int i = 0; i < 3; ++i)
		{
			for (int j = -2; j < 20; ++j)
			{
				if ((i + j) % 3 != 0) continue;

				float scale = Random::Range(1.0f, 1.5f);

				scene->Instantiate<Tree>(
					"Tree_" + std::to_string(i * 10 + j),
					Transform{
						{0.0f + (2.0f) * j, 2.0f, 40.0f + (2.0f) * i},
						{0.0f, 0.0f, 0.0f}, {0.01f * scale, 0.01f * scale, 0.01f * scale}
					},
					true,
					E_RenderLayer::Opaque);
				scene->Instantiate<Tree>(
					"Tree_" + std::to_string(i * 10 + j),
					Transform{
						{0.0f + (2.0f) * j, 2.0f, -42.0f - (2.0f) * i},
						{0.0f, 0.0f, 0.0f}, {0.01f * scale, 0.01f * scale, 0.01f * scale}
					},
					true,
					E_RenderLayer::Opaque);
			}
		}

	}

	// create trees (house side)
	{
		int radius = 13;

		for (int i = -20; i < 0; ++i)
		{
			for (int j = -22; j < 22; ++j)
			{
				if (i * i + j * j < radius * radius) continue;
				if ((i * i + j * j) % (std::abs(j) + 1) >= std::abs(j) / 4) continue;
				if (i == -1 && j % 5 != 0) continue;

				float scale = Random::Range(1.0f, 1.5f);

				scene->Instantiate<Tree>(
					"Tree_" + std::to_string(i * 10 + j),
					Transform{
						{0.0f + (2.0f) * i, 2.0f, 0.0f + (2.0f) * j},
						{0.0f, 0.0f, 0.0f}, {0.01f * scale, 0.01f * scale, 0.01f * scale}
					},
					true,
					E_RenderLayer::Opaque);
			}
		}
	}

	// create trees (out bound)
	{
		for (int i = -25; i < 25; ++i)
		{
			scene->Instantiate<Tree>(
					"tree_bound",
					Transform{
						{50.0f, 2.0f, 0.0f + (2.0f) * i},
						{0.0f, 0.0f, 0.0f}, {0.013f, 0.013f, 0.013f}
					},
					true,
					E_RenderLayer::Opaque);
			
			scene->Instantiate<Tree>(
				"tree_bound",
				Transform{
					{-50.0f, 2.0f, 0.0f + (2.0f) * i},
					{0.0f, 0.0f, 0.0f}, {0.013f, 0.013f, 0.013f}
				},
				true,
				E_RenderLayer::Opaque);

			scene->Instantiate<Tree>(
				"tree_bound",
				Transform{
					{0.0f + (2.0f) * i, 2.0f, 50.0f},
					{0.0f, 0.0f, 0.0f}, {0.013f, 0.013f, 0.013f}
				},
				true,
				E_RenderLayer::Opaque);

			scene->Instantiate<Tree>(
				"tree_bound",
				Transform{
					{0.0f + (2.0f) * i, 2.0f, -50.0f},
					{0.0f, 0.0f, 0.0f}, {0.013f, 0.013f, 0.013f}
				},
				true,
				E_RenderLayer::Opaque);
		}
	}

	

}

void DirectXGame::OnKeyDown(UINT8 key)
{
	m_currentScene->GetCamera()->OnKeyDown(key);
}

void DirectXGame::OnKeyUp(UINT8 key)
{
	m_currentScene->GetCamera()->OnKeyUp(key);
}

void DirectXGame::OnMouseDown(UINT8 btnState, int x, int y)
{
	m_player->OnMouseDown(btnState, x, y);
}

void DirectXGame::OnMouseUp(UINT8 btnState, int x, int y)
{
	m_player->OnMouseUp(btnState, x, y);
}

void DirectXGame::OnMouseMove(UINT8 btnState, int x, int y)
{
	m_currentScene->GetCamera()->OnMouseMove(btnState, x, y);
}

void DirectXGame::OnMouseLeave(UINT8 btnState, int x, int y)
{
	RECT window;
	GetClientRect(Win32Application::GetHwnd(), &window);
	
	POINT center;
	center.x = (window.left + window.right) / 2;
	center.y = (window.top + window.bottom) / 2;

	m_currentScene->GetCamera()->ResetMousePos(center);
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

void Scene::BuildObject()
{
	// create and initialize game objects.


	// init camera
	{
		m_camera.SetPosition(XMFLOAT3(-15.0f, 5.0f, 5.0f));
		m_camera.SetRotation(XMFLOAT3(30.0f, 90.0f, 0.0f));
		m_camera.SetFrustum(XM_PI * 0.25f);
	}
}

void Scene::Update(float dt)
{
	// assign instantiated objects at prev frame.
	AssignInstantiatedObjects();

	// update all objects.
	DirectXGame::GetPlayer()->Update(dt);
	for (auto& obj : m_objects)
	{
		if (obj->IsActive()) obj->Update(dt);
	}

	m_camera.Update(dt);
	
	// update instance buffers.
	UpdateInstanceData();
}

void Scene::AssignInstantiatedObjects()
{
	while (!m_queue.empty())
	{
		auto front = std::move(m_queue.front());
		m_queue.pop();

		front->SetBufferId((UINT)m_objects.size());
		front->SetDirty();

		int layer = front->GetLayer();
		

		if (layer >= 0)
		{
			if (layer < static_cast<int>(E_RenderLayer::Count)) m_GameObjects[layer].push_back(reinterpret_cast<GameObject*>(front.get()));
			else m_UIObjects.push_back(reinterpret_cast<UIObject*>(front.get()));
		}

		m_objects.push_back(std::move(front));

	}
}

std::vector<GameObject*> Scene::GetAllGameObjects()
{
	std::vector<GameObject*> result;

	for (auto layer : m_GameObjects)
	{
		for (auto& obj : layer.second)
		{
			result.push_back(obj);
		}
	}

	return result;
}

void Scene::UpdateInstanceData()
{
	auto gameObjects = GetAllGameObjects();

	// cull test
	for (auto& obj : gameObjects)
	{
		// root object만 테스트
		if (obj->GetParentObject() != nullptr) continue;
		
		// active object 대상으로 cull test
		if (!obj->IsActive()) continue;
		bool culltest = obj->IsCullingEnabled() ? m_camera.FrustumCullTest(obj) : true;
		obj->SetCullTested(culltest);
	}


	for (auto& obj : gameObjects)
	{
		// cull test 결과가 동일하고, dirty flag도 없다면 업데이트 하지 않는다.
		if (!obj->IsDirty() && (obj->IsCulledLastFrame() == obj->IsCulledNextFrame())) continue;

		InstanceData data;
		XMStoreFloat4x4(&data.model, XMMatrixTranspose(obj->GetWorldMatrix()));
		data.matIndex = obj->GetRenderer()->GetMaterialIndex();
		data.layer = obj->GetLayer();		
		data.active = obj->IsActive() && obj->IsCulledNextFrame();
		
		obj->GetRenderer()->AssignInstance(data);
		obj->SetDirty(false);
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
		tex->id = 0;
		
		m_textures[tex->name] = std::move(tex);
	}


	// Load meshes with textures.
	{
		std::vector<std::string> fbxName = {
			"Assets/Pen and Notepad.fbx",
			"Assets/pillow.fbx",
			"Assets/bed.fbx",
			"Assets/table.fbx",
			"Assets/house.fbx",
			"Assets/carrot.fbx",
			"Assets/cratebox.fbx",
			"Assets/tree.fbx",
		};

		FbxLoader loader = FbxLoader(&Assets::m_textures, &Assets::m_materials);

		for (auto a : fbxName)
		{
			if (m_models.find(a) == m_models.end()) {
				Mesh mesh;
				loader.Load(a.c_str(), &mesh);
				m_models[a] = mesh;

				auto meshDesc = std::make_unique<MeshDesc>();
				meshDesc->id = m_meshes.size();
				meshDesc->mesh = &m_models[a];
				meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());
				m_meshes[a] = std::move(meshDesc);
			}
		}
	
	
		auto grass = std::make_unique<Texture>();
		grass->name = "grass_tile";
		grass->filePath = L"Textures/custom/grass_tile.dds";
		grass->id = m_textures.size();
		grass->type = E_TextureType::Texture2D;
		m_textures[grass->name] = std::move(grass);
	
		auto dirt = std::make_unique<Texture>();
		dirt->name = "dirt_tile";
		dirt->filePath = L"Textures/custom/dirt_tile.dds";
		dirt->id = m_textures.size();
		dirt->type = E_TextureType::Texture2D;
		m_textures[dirt->name] = std::move(dirt);

		auto field_mat = std::make_unique<Material>();
		field_mat->name = "field-mat";
		field_mat->diffuseMapIndex = m_textures["dirt_tile"]->id;
		field_mat->bufferId = m_materials.size();
		m_materials[field_mat->name] = std::move(field_mat);


		auto note = std::make_unique<Texture>();
		note->name = "note";
		note->filePath = L"Textures/custom/Notepad_diffuse.dds";
		note->id = m_textures.size();
		note->type = E_TextureType::Texture2D;
		m_textures[note->name] = std::move(note);

		auto note_mat = std::make_unique<Material>();
		note_mat->name = "note-mat";
		note_mat->diffuseMapIndex = m_textures["note"]->id;
		note_mat->bufferId = m_materials.size();
		m_materials[note_mat->name] = std::move(note_mat);



		auto tree_mat = std::make_unique<Material>();
		tree_mat->name = "tree-mat";
		tree_mat->diffuseMapIndex = 0;
		tree_mat->diffuseColor = { 76.0f / 255.0f, 135.0f / 255.0f, 27.0f / 255.0f };
		tree_mat->bufferId = m_materials.size();
		m_materials[tree_mat->name] = std::move(tree_mat);

		auto carrot_mat = std::make_unique<Material>();
		carrot_mat->name = "carrot-mat";
		carrot_mat->diffuseMapIndex = 0;
		carrot_mat->diffuseColor = { 254.0f / 255.0f, 162.0f / 255.0f, 23.0f / 255.0f };
		carrot_mat->bufferId = m_materials.size();
		m_materials[carrot_mat->name] = std::move(carrot_mat);
	}


	// Load bitmap images.
	{
		std::vector<std::string> imgName
		{
			"cross-aim",
			"default-black",
			"default-white",
			"paper",
		};
		
		std::vector<std::wstring> imgPath
		{
			L"Sprites/aim.png",
			L"Sprites/default/black.jpg",
			L"Sprites/default/white.jpg",
			L"Sprites/paper.png",
		};

		for (UINT i = 0; i < imgName.size(); ++i)
		{
			if (m_sprites.find(imgName[i]) == m_sprites.end()) {
				auto sprite = std::make_unique<Sprite>();
				sprite->name = imgName[i];
				sprite->filePath = imgPath[i];
				sprite->id = i;

				m_sprites[sprite->name] = std::move(sprite);
			}
		}	
	}


	// Create the default skybox material.
	{
		auto tex = std::make_unique<Texture>();
		tex->name = "skybox";
		tex->filePath = L"Textures/sky/grasscube1024.dds";
		tex->id = m_textures.size();
		tex->type = E_TextureType::TextureCube;

		m_textures[tex->name] = std::move(tex);

		auto mat = std::make_unique<Material>();
		mat->name = "sky";
		mat->diffuseMapIndex = m_textures["skybox"]->id;
		mat->bufferId = m_materials.size();

		m_materials[mat->name] = std::move(mat);
	}

	// create default text brushes.
	{
		auto lead = std::make_unique<TextDesc>();
		lead->brushColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
		lead->fontSize = 20;
		lead->id = m_texts.size();
		m_texts["black-leading"] = std::move(lead);

		auto center = std::make_unique<TextDesc>();
		center->brushColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
		center->textAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
		center->fontSize = 20;
		center->id = m_texts.size();
		m_texts["black-center"] = std::move(center);
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
		meshDesc->id = m_meshes.size();
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["triangle"] = std::move(meshDesc);
	}

	// create default plane
	{
		Mesh mesh;
		mesh.vertices = {
			Vertex {{ -1.0f, 0.0f, -1.0f }, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ -1.0f, 0.0f, 1.0f }, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ 1.0f, 0.0f, 1.0f }, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ 1.0f, 0.0f, -1.0f }, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
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
		meshDesc->id = m_meshes.size();
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["plane"] = std::move(meshDesc);
	}

	// create default sphere.
	{
		float radius = 0.5f;
		uint16_t sliceCount = 20;
		uint16_t stackCount = 20;


		Mesh mesh;

		Vertex top = {{ 0.0f, +radius, 0.0f }, { 0.0f, +1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }};
		Vertex bottom = { {0.0f, -radius, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} , {1.0f, 0.0f, 0.0f} };

		mesh.vertices.push_back(top);

		float phiStep = XM_PI / stackCount;
		float thetaStep = 2.0f * XM_PI / sliceCount;

		// Compute vertices for each stack ring (do not count the poles as rings).
		for (uint16_t i = 1; i <= stackCount - 1; ++i)
		{
			float phi = i * phiStep;

			// Vertices of ring.
			for (uint16_t j = 0; j <= sliceCount; ++j)
			{
				float theta = j * thetaStep;

				Vertex v;

				// spherical to cartesian
				v.position.x = radius * sinf(phi) * cosf(theta);
				v.position.y = radius * cosf(phi);
				v.position.z = radius * sinf(phi) * sinf(theta);

				// Partial derivative of P with respect to theta
				v.tangent.x = -radius * sinf(phi) * sinf(theta);
				v.tangent.y = 0.0f;
				v.tangent.z = +radius * sinf(phi) * cosf(theta);

				XMVECTOR T = XMLoadFloat3(&v.tangent);
				XMStoreFloat3(&v.tangent, XMVector3Normalize(T));

				XMVECTOR p = XMLoadFloat3(&v.position);
				XMStoreFloat3(&v.normal, XMVector3Normalize(p));

				v.uv.x = theta / XM_2PI;
				v.uv.y = phi / XM_PI;

				mesh.vertices.push_back(v);
			}
		}

		mesh.vertices.push_back(bottom);

		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		for (uint16_t i = 1; i <= sliceCount; ++i)
		{
			mesh.indices.push_back(0);
			mesh.indices.push_back(i + 1);
			mesh.indices.push_back(i);
		}

		//
		// Compute indices for inner stacks (not connected to poles).
		//

		// Offset the indices to the index of the first vertex in the first ring.
		// This is just skipping the top pole vertex.
		uint16_t baseIndex = 1;
		uint16_t ringVertexCount = sliceCount + 1;
		for (uint16_t i = 0; i < stackCount - 2; ++i)
		{
			for (uint16_t j = 0; j < sliceCount; ++j)
			{
				mesh.indices.push_back(baseIndex + i * ringVertexCount + j);
				mesh.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

				mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
				mesh.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
			}
		}

		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		uint16_t southPoleIndex = (uint16_t)mesh.vertices.size() - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexCount;

		for (uint16_t i = 0; i < sliceCount; ++i)
		{
			mesh.indices.push_back(southPoleIndex);
			mesh.indices.push_back(baseIndex + i);
			mesh.indices.push_back(baseIndex + i + 1);
		}

		m_models["sphere"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["sphere"];
		meshDesc->id = m_meshes.size();
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["sphere"] = std::move(meshDesc);
	}

	// create default quad.
	{
		Mesh mesh;
		float x = 0.0f;
		float y = 0.0f;
		float w = 1.0f;
		float h = 1.0f;
		float depth = 0.0f;

		mesh.vertices = {
			Vertex {{ x, y - h, depth }, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ x, y, depth }, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ x + w, y, depth }, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex {{ x + w, y - h, depth }, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		};

		mesh.indices = {
			0, 1, 2,
			0, 2, 3
		};

		mesh.matIndex.push_back(0);

		m_models["quad"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["quad"];
		meshDesc->id = m_meshes.size();
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["quad"] = std::move(meshDesc);
	}

	// create default box.
	{
		Mesh mesh;
		float w = 0.5f;
		float h = 0.5f;
		float d = 0.5f;
		mesh.vertices = 
		{
			Vertex{{-w, -h, -d}, {0.0f, 0.0f, -1.0}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex{{-w, +h, -d}, {0.0f, 0.0f, -1.0}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex{{+w, +h, -d}, {0.0f, 0.0f, -1.0}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex{{+w, -h, -d}, {0.0f, 0.0f, -1.0}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

			Vertex{{-w, -h, +d}, {0.0f, 0.0f, +1.0}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
			Vertex{{+w, -h, +d}, {0.0f, 0.0f, +1.0}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
			Vertex{{+w, +h, +d}, {0.0f, 0.0f, +1.0}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
			Vertex{{-w, +h, +d}, {0.0f, 0.0f, +1.0}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},

			Vertex{{-w, +h, -d}, {0.0f, 1.0f, 0.0}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex{{-w, +h, +d}, {0.0f, 1.0f, 0.0}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex{{+w, +h, +d}, {0.0f, 1.0f, 0.0}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			Vertex{{+w, +h, -d}, {0.0f, 1.0f, 0.0}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

			Vertex{{-w, -h, -d}, {0.0f, -1.0f, 0.0}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
			Vertex{{+w, -h, -d}, {0.0f, -1.0f, 0.0}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
			Vertex{{+w, -h, +d}, {0.0f, -1.0f, 0.0}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
			Vertex{{-w, -h, +d}, {0.0f, -1.0f, 0.0}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},

			Vertex{{-w, -h, +d}, {-1.0f, 0.0f, 0.0}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
			Vertex{{-w, +h, +d}, {-1.0f, 0.0f, 0.0}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
			Vertex{{-w, +h, -d}, {-1.0f, 0.0f, 0.0}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
			Vertex{{-w, -h, -d}, {-1.0f, 0.0f, 0.0}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},

			Vertex{{+w, -h, -d}, {1.0f, 0.0f, 0.0}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			Vertex{{+w, +h, -d}, {1.0f, 0.0f, 0.0}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			Vertex{{+w, +h, +d}, {1.0f, 0.0f, 0.0}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			Vertex{{+w, -h, +d}, {1.0f, 0.0f, 0.0}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		};

		/*mesh.indices =
		{
			0, 1, 2,
			0, 2, 3,
			4, 5, 6,
			4, 6, 7,
			8, 9, 10,
			8, 10, 11,
			12, 13, 14,
			12, 14, 15,
			16, 17, 18,
			16, 18, 19,
			20, 21, 22,
			20, 22, 23
		};*/

		// D3D_PRIMITIVE_TOPOLOGY_LINESTRIP indices
		// https://docs.microsoft.com/ko-kr/windows/uwp/graphics-concepts/primitive-topologies
		mesh.indices =
		{
			0, 1, 2, 3,
			12, 13, 14, 15,
			19, 18, 17, 16,
			7, 4, 5, 6,
			21, 20, 23, 22,
			9, 8, 11, 10
		};

		mesh.matIndex.push_back(0);

		m_models["box"] = mesh;

		auto meshDesc = std::make_unique<MeshDesc>();
		meshDesc->mesh = &m_models["box"];
		meshDesc->id = m_meshes.size();
		meshDesc->indexCount = (UINT)(meshDesc->mesh->indices.size());

		m_meshes["box"] = std::move(meshDesc);
	}
}

Material* Assets::GetMaterialByID(UINT id)
{
	for (auto& m : m_materials)
	{
		if (m.second->bufferId == id) {
			return m.second.get();
		}
	}

	return nullptr;
}

std::vector<Texture*> Assets::GetOrderedTextures()
{
	std::vector<Texture*> vector;

	for (auto& i : m_textures)
	{
		vector.push_back(i.second.get());
	}

	std::sort(vector.begin(), vector.end(), [](Texture* a, Texture* b) {
		return a->id < b->id;
		});

	return vector;
}

std::vector<MeshDesc*> Assets::GetMeshDesc()
{
	std::vector<MeshDesc*> vector;
	
	for (auto& i : m_meshes)
	{
		vector.push_back(i.second.get());
	}

	return vector;
}

std::vector<Sprite*> Assets::GetSprites()
{
	std::vector<Sprite*> vector;

	for (auto& i : m_sprites)
	{
		vector.push_back(i.second.get());
	}

	return vector;
}


std::vector<TextDesc*> Assets::GetTexts()
{
	std::vector<TextDesc*> vector;

	for (auto& i : m_texts)
	{
		vector.push_back(i.second.get());
	}

	return vector;
}