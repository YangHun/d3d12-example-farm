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
			
			AssignIndexedVertex(vertex);
		}
	}
}

void FbxLoader::AssignIndexedVertex(Vertex vertex)
{
	if (m_indexMap.find(vertex) == m_indexMap.end()) {
		m_indexMap[vertex] = (uint16_t)m_dstData->vertices.size();
		m_dstData->vertices.push_back(vertex);
	}
	m_dstData->indices.push_back(m_indexMap[vertex]);
}

void FbxLoader::GetPosition(FbxMesh* mesh, int pIndex, XMFLOAT3& vertexData)
{
	FbxVector4 fbxData = mesh->GetControlPointAt(pIndex);

	fbxData = m_globalTransform.MultT(fbxData);
	
	
	FbxAMatrix scale;
	scale.SetIdentity();
	scale.SetS(FbxVector4{ 0.001f, 0.001f, 0.001f, 1.0f });
	fbxData = scale.MultT(fbxData);
	

	vertexData = Fbx4ToXM3(fbxData);


}
