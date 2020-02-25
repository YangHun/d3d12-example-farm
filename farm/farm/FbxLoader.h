#pragma once

#ifndef _FBX_LOADER_H
#define _FBX_LOADER_H

class FbxLoader
{
public:
	FbxLoader();

	bool Load(const char* filename, Mesh* mesh);

private:
	void LoadNode(FbxNode* node);

	void GetGlobalTransform(FbxNode* node);
	void ProcessControlPoints(FbxMesh* mesh);
	void GetPosition(FbxMesh* mesh, int pIndex, XMFLOAT3& vertexData);
	//void TryGetNormal();
	//void TryGetUV();
	//void TryGetTangent();
	void AssignIndexedVertex(Vertex v);

private:
	FbxManager* m_manager = nullptr;
	FbxIOSettings* m_ios = nullptr;
	FbxImporter* m_importer = nullptr;

	// global variables for parsing data
	Mesh* m_dstData = nullptr;
	FbxAMatrix m_globalTransform;
	std::unordered_map<Vertex, uint16_t> m_indexMap;
};


// helper functions

inline XMFLOAT3 Fbx4ToXM3(FbxVector4 fbx)
{
	return XMFLOAT3(
		static_cast<float>(fbx.mData[0]),
		static_cast<float>(fbx.mData[1]),
		static_cast<float>(fbx.mData[2])
	);
}

inline XMFLOAT4 Fbx4ToXM4(FbxVector4 fbx)
{
	return XMFLOAT4(
		static_cast<float>(fbx.mData[0]),
		static_cast<float>(fbx.mData[1]),
		static_cast<float>(fbx.mData[2]),
		static_cast<float>(fbx.mData[3])
	);
}

#endif