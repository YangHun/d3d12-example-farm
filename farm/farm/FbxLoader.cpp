#include "stdafx.h"
#include "FbxLoader.h"

FbxLoader::FbxLoader()
{
	if (!m_manager) m_manager = FbxManager::Create();
	if (!m_ios) m_ios = FbxIOSettings::Create(m_manager, IOSROOT);
	m_manager->SetIOSettings(m_ios);
	if (!m_importer) m_importer = FbxImporter::Create(m_manager, "");
	m_globalTransform.SetIdentity();
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
	m_indexMap.clear();
	LoadNode(rootNode);

	return true;
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

	FbxAMatrix scale;
	scale.SetIdentity();
	scale.SetT(FbxVector4(0.0f, -0.5f, 0.5f, 1.0f));
	scale.SetS(FbxVector4(0.001f, 0.001f, 0.001f, 1.0f));
	fbxData = scale.MultT(fbxData);

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
}