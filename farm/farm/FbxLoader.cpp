#include "stdafx.h"
#include "FbxLoader.h"

FbxLoader::FbxLoader(std::unordered_map<std::string, std::unique_ptr<Texture>>* pTextureMap, std::unordered_map<std::string, std::unique_ptr<Material>>* pMaterialMap) :
	m_pTextures(pTextureMap),
	m_pMaterials(pMaterialMap)
{
	if (!m_manager) m_manager = FbxManager::Create();
	if (!m_ios) m_ios = FbxIOSettings::Create(m_manager, IOSROOT);
	m_manager->SetIOSettings(m_ios);
	if (!m_importer) m_importer = FbxImporter::Create(m_manager, "");
	m_globalTransform.SetIdentity();
}

FbxLoader::~FbxLoader()
{
	if (m_importer != nullptr)
	{
		m_importer->Destroy();
		m_importer = nullptr;
	}
	
	if (m_ios != nullptr)
	{
		m_ios->Destroy();
		m_ios = nullptr;
	}

	if (m_manager != nullptr)
	{
		m_manager->Destroy();
		m_manager = nullptr;
	}


	m_indexMap.clear();
	m_texMap.clear();
}

bool FbxLoader::Load(const char* filename, Mesh* meshOutput)
{
	if (!m_importer) m_importer = FbxImporter::Create(m_manager, "");

	bool status = m_importer->Initialize(filename, -1, m_manager->GetIOSettings());
	if (!status)
	{
		return false;
	}

	// load FbxScene
	FbxScene* scene = FbxScene::Create(m_manager, "scene");
	m_importer->Import(scene);
	m_importer->Destroy();
	m_importer = nullptr;

	FbxNode* rootNode = scene->GetRootNode();
	FbxAxisSystem sceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();


	FbxGeometryConverter* geometryConverter = new FbxGeometryConverter(m_manager);
	// triangulate all nodes that could be changed to triangle.
	geometryConverter->Triangulate(scene, true);
	delete(geometryConverter);

	m_dstData = meshOutput;

	GetTexturesPath(scene, filename);
	BuildMaterials(scene);

	m_indexMap.clear();
	LoadNode(rootNode);

	return true;
}

void FbxLoader::GetTexturesPath(FbxScene* scene, const char* filename)
{
	int count = scene->GetTextureCount();

	for (int i = 0; i < count; ++i)
	{
		FbxTexture* fbx = scene->GetTexture(i);
		
		if (m_pTextures->find(fbx->GetName()) == m_pTextures->end()) {

			auto data = std::make_unique<Texture>();

			data->name = fbx->GetName();
			data->filePath = ConvertTexturePath(filename, data->name);

			data->id = m_pTextures->size();
			m_pTextures->insert({ data->name, std::move(data) });
		}
	}
}

std::wstring FbxLoader::ConvertTexturePath(const char* mesh, std::string texture)
{
	std::string meshName (mesh);
	meshName = meshName.substr(7, meshName.size() - (4 + 7));	// exclude 7(Assets/) + 4(.fbx)

	std::string textureName = texture;
	if (textureName.find("_jpg") != std::string::npos)
	{
		auto lookup = textureName.find("_jpg");
		textureName = textureName.replace(lookup, lookup + 4, ".dds");
	}

	if (textureName.find(".dds") == std::string::npos)
	{
		textureName.append(".dds");
	}

	std::stringstream ss;
	ss << "Textures/" << meshName << "/" << textureName;

	std::string str = ss.str();
	std::wstring wstr;
	wstr.assign(str.begin(), str.end());

	return wstr;
}

void FbxLoader::BuildMaterials(FbxScene* scene)
{
	int count = scene->GetMaterialCount();

	for (int i = 0; i < count; ++i)
	{
		FbxSurfaceMaterial* fMat = scene->GetMaterial(i);
		FbxClassId cid = fMat->GetClassId();
		auto prop = fMat->FindProperty(FbxSurfaceMaterial::sDiffuse);
		
		if (m_pMaterials->find(fMat->GetName()) != m_pMaterials->end()) continue;

		auto mat = std::make_unique<Material>();
		FbxTexture* tex = prop.GetSrcObject<FbxTexture>(0);
		
		if (tex != nullptr)
		{
			//std::string n1 = tex->GetInitialName();
			//std::string n2 = tex->GetName();
			UINT index = m_pTextures->at(tex->GetName())->id;
			mat->diffuseMapIndex = index;
		}
		else {
			mat->diffuseMapIndex = 0;	// default texture
		}

		if (cid.Is(FbxSurfacePhong::ClassId))
		{
			FbxSurfacePhong* phong = (FbxSurfacePhong*)(fMat);
			mat->name = phong->GetName();
		}
		else if (cid.Is(FbxSurfaceLambert::ClassId))
		{
			FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)(fMat);
			mat->name = lambert->GetName();
		}

		mat->bufferId = (UINT)m_pMaterials->size();
		m_dstData->matIndex.push_back(mat->bufferId);
		m_pMaterials->insert({ mat->name, std::move(mat) });		
	}
}

void FbxLoader::LoadNode(FbxNode* node)
{
	FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();

	if (nodeAttribute)
	{
		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			GetGlobalTransform(node);
			ProcessControlPoints(node->GetMesh());
		}
	}

	int childCount = node->GetChildCount();
	for (size_t i = 0; i < childCount; ++i)
	{
		LoadNode(node->GetChild((int)i));
	}

}

void FbxLoader::GetGlobalTransform(FbxNode* node)
{
	m_globalTransform.SetIdentity();
	const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);
	m_globalTransform.SetT(lT);
	m_globalTransform.SetR(lR);
	m_globalTransform.SetS(lS);

	m_globalTransform = node->EvaluateGlobalTransform() * m_globalTransform;
	node->EvaluateGlobalTransform();
}


void FbxLoader::ProcessControlPoints(FbxMesh* mesh)
{
	int count = mesh->GetPolygonCount();

	// all surfaces are triangulated.
	for (int i = 0; i < count; ++i)
	{
		for (int j = 0; j < 3; ++j) {

			Vertex vertex;
			int polygonIndex = mesh->GetPolygonVertex(i, j);

			GetPosition(mesh, polygonIndex, vertex.position);
			TryGetNormal(mesh, i, j, vertex.normal);
			TryGetUV(mesh, i, j, vertex.uv);
			TryGetTangent(mesh, i, j, vertex.tangent);

			AssignIndexedVertex(vertex);
		}
	}
}

void FbxLoader::GetPosition(FbxMesh* mesh, int pIndex, XMFLOAT3& vertexData)
{
	FbxVector4 fbxData = mesh->GetControlPointAt(pIndex);
	fbxData = m_globalTransform.MultT(fbxData);
	
	vertexData = Fbx4ToXM3(fbxData);
}

void FbxLoader::TryGetNormal(FbxMesh* mesh, int pCount, int vCount, XMFLOAT3& vertexData)
{
	FbxVector4 normal;
	bool bResult = mesh->GetPolygonVertexNormal(pCount, vCount, normal);

	normal.Normalize();

	vertexData = (bResult) ? Fbx4ToXM3(normal) : XMFLOAT3(0.0f, 1.0f, 0.0f);

}


void FbxLoader::TryGetUV(FbxMesh* mesh, int pCount, int vCount, XMFLOAT2& vertexData)
{
	FbxStringList uvNames;
	mesh->GetUVSetNames(uvNames);

	if (uvNames.GetCount() < 1) {
		vertexData = XMFLOAT2(0.0f, 0.0f);
		return;
	}

	FbxVector2 uv;
	bool bUnmapped;
	bool bResult = mesh->GetPolygonVertexUV(pCount, vCount, uvNames[0], uv, bUnmapped);

	vertexData = (bResult) ? Fbx2ToXM2(uv) : XMFLOAT2(0.0f, 0.0f);

	// DirectX 의 UV 기준점은 왼쪽 위, Fbx 의 UV 기준점은 왼쪽 아래
	// v 값에 -1을 곱해 맞춰준다.
	vertexData.y *= -1.0f;
}


void FbxLoader::TryGetTangent(FbxMesh* mesh, int pIndex, int vIndex, XMFLOAT3& vertexData)
{
	if (mesh->GetElementTangentCount() < 1)
	{
		// re-calculate tangent for uv layer
		mesh->GenerateTangentsDataForAllUVSets(true, false);
	}

	int count = mesh->GetElementTangentCount();

	if (count < 1)
	{
		vertexData = XMFLOAT3(0.0f, 0.0f, 0.0f);
		return;
	}

	const FbxGeometryElementTangent* elementTangent = mesh->GetElementTangent(0);

	FbxVector4 tangent;

	if (elementTangent->GetMappingMode() == FbxGeometryElement::eByControlPoint)
	{
		tangent = (elementTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
			? elementTangent->GetDirectArray().GetAt(pIndex)
			: elementTangent->GetDirectArray().GetAt(elementTangent->GetIndexArray().GetAt(pIndex));

	}
	else if (elementTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
	{
		tangent = (elementTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
			? elementTangent->GetDirectArray().GetAt(vIndex)
			: elementTangent->GetDirectArray().GetAt(elementTangent->GetIndexArray().GetAt(vIndex));
	}

	tangent.Normalize();

	vertexData = Fbx4ToXM3(tangent);
}

void FbxLoader::AssignIndexedVertex(Vertex vertex)
{
	if (m_indexMap.find(vertex) == m_indexMap.end()) {
		m_indexMap[vertex] = (uint16_t)m_dstData->vertices.size();
		m_dstData->vertices.push_back(vertex);
	}
	m_dstData->indices.push_back(m_indexMap[vertex]);

	UpdateBound(vertex.position);
}

void FbxLoader::UpdateBound(XMFLOAT3 position)
{

	if (position.x < m_dstData->minBound.x)  m_dstData->minBound.x = position.x;
	else if (position.x > m_dstData->maxBound.x)  m_dstData->maxBound.x = position.x;

	if (position.y < m_dstData->minBound.y)  m_dstData->minBound.y = position.y;
	else if (position.y > m_dstData->maxBound.y)  m_dstData->maxBound.y = position.y;

	if (position.z < m_dstData->minBound.z)  m_dstData->minBound.z = position.z;
	else if (position.z > m_dstData->maxBound.z)  m_dstData->maxBound.z = position.z;
}