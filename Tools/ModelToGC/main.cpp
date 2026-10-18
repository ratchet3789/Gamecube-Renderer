#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Types.h"

using namespace std;

bool IsFileSupportedFormat(const char* Arg);
bool IsOpenGL(const char* Arg);

void ProcessArguments(int args, char *argv[]);
bool Import(const std::string& pFile);
bool ExtractMeshData(aiMesh** MeshArray, int MeshCount, std::string FilePath);
void ExportMeshToFile(const char* FilePath, const uint8_t* HeaderBlob, const size_t TotalBufferSize);

bool g_bUsesOpenGL{false};

// Flip UINT unless we're on OpenGL
uint32_t FlipUINT(const uint32_t in)
{
	if (g_bUsesOpenGL)
		return in;

	return ((0xFF000000 & in) >> 24 | // Move 3 > 0
		(0x00FF0000 & in) >> 8  | // Move 2 > 1
		(0x0000FF00 & in) << 8  | // Move 1 < 2
		(0x000000FF & in) << 24); // Move 0 < 3
}

uint16_t FlipUSHORT(const uint16_t in)
{
	if (g_bUsesOpenGL)
		return in;

	return ((0x00FF0000 & in) >> 8  | // Move 2 > 1
		(0x0000FF00 & in) << 8); // Move 1 < 2
}

float FlipFLOAT(const float in)
{
	if (g_bUsesOpenGL)
		return in;

	uint32_t bits;
	float out = 0.f;
	std::memcpy(&bits, &in, sizeof(float));
	bits = FlipUINT(bits);
	std::memcpy(&out, &bits, sizeof(float));
	return out;
}

int main(int argc, char *argv[])
{
	ProcessArguments(argc, argv);
	return 0;
}

void ProcessArguments(int args, char *argv[])
{
	// Ignore 0 (exe)
	for (int i = 1; i < args; i++)
	{
		if (IsOpenGL(argv[i]))
		{
			g_bUsesOpenGL=true;
			continue;
		}

		if (IsFileSupportedFormat(argv[i]))
		{
			Import(argv[i]);
			continue;
		}
	}
}

bool Import(const std::string& pFile)
{
	Assimp::Importer Importer;
	const aiScene* pScene = Importer.ReadFile(pFile,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (pScene == nullptr)
	{
		return false;
	}

	return ExtractMeshData(pScene->mMeshes, pScene->mNumMeshes, pFile);
}

bool ExtractMeshData(aiMesh** MeshArray, int MeshCount, std::string FilePath)
{
	if (MeshCount == 0)
		return false;

	for (int i = 0; i < MeshCount; i++)
	{
		aiMesh*& Mesh = MeshArray[i];
		StaticMesh SM = StaticMesh();

		if (g_bUsesOpenGL)
		{
			// I genuinely hate this with a passion I cannot describe to you
			char newType[4] = {'G','L','S','M'};
			memcpy(SM.TypeIdentifier, newType, sizeof(SM.TypeIdentifier));
		}

		// Get the count of each object and setup our Vectors
		const uint32_t IndexSize = Mesh->mNumFaces;
		const uint32_t VertexSize = Mesh->mNumVertices;
		const uint32_t TextureSize = Mesh->GetNumUVChannels();
		const uint32_t NormalSize = Mesh->mNumVertices;
		const uint32_t ColorSize = Mesh->GetNumColorChannels();

		std::vector<Vec3i> Indices{};
		std::vector<Vec3f> Verts{};
		std::vector<Vec3f> Textures{};
		std::vector<Vec3f> Normals{};
		std::vector<Vec4f> Colors{};

		// Extract the data from all of our channels, store it in our 16 byte aligned VectorXX files and Flip it for Endian-ness
		for (uint32_t i = 0; i < IndexSize; i++)
		{
			Vec3i idx3{};
			idx3.x = FlipUINT(Mesh->mFaces[i].mIndices[0]);
			idx3.y = FlipUINT(Mesh->mFaces[i].mIndices[1]);
			idx3.z = FlipUINT(Mesh->mFaces[i].mIndices[2]);
			Indices.push_back(idx3);
		}

		for (uint32_t i = 0; i < VertexSize; i++)
		{
			Vec3f vtx3{};
			vtx3.x = FlipFLOAT(Mesh->mVertices[i].x);
			vtx3.y = FlipFLOAT(Mesh->mVertices[i].y);
			vtx3.z = FlipFLOAT(Mesh->mVertices[i].z);
			Verts.push_back(vtx3);
		}

		Mesh->mTextureCoords;
		for (uint32_t i = 0; i < TextureSize; i++)
		{
			Vec3f uv3{};
			uv3.x = FlipFLOAT(Mesh->mTextureCoords[i]->x);
			uv3.y = FlipFLOAT(Mesh->mTextureCoords[i]->y);
			uv3.z = FlipFLOAT(Mesh->mTextureCoords[i]->z);
			Textures.push_back(uv3);
		}

		Mesh->mNormals;
		for (uint32_t i = 0; i < NormalSize; i++)
		{
			if (Mesh->mNormals == NULL)
				continue;

			Vec3f nrm3{};
			nrm3.x = FlipFLOAT(Mesh->mNormals[i].x);
			nrm3.y = FlipFLOAT(Mesh->mNormals[i].y);
			nrm3.z = FlipFLOAT(Mesh->mNormals[i].z);
			Normals.push_back(nrm3);
		}

		Mesh->mColors;
		for (uint32_t i = 0; i < ColorSize; i++)
		{
			Vec4f col4{};
			col4.x = FlipFLOAT(Mesh->mColors[i]->r);
			col4.y = FlipFLOAT(Mesh->mColors[i]->g);
			col4.z = FlipFLOAT(Mesh->mColors[i]->b);
			col4.w = FlipFLOAT(Mesh->mColors[i]->a);
			Colors.push_back(col4);
		}

		// Now store it in our Header as specific data offset by the Count * Sizeof
		int LastOffset = sizeof(StaticMesh);

		SM.IndexCount = FlipUINT((uint32_t)Indices.size());
		SM.IndexArrayOffset = FlipUINT(LastOffset);
		LastOffset += FlipUINT(SM.IndexCount) * sizeof(Vec3i);

		SM.VertexCount = FlipUINT((uint32_t)Verts.size());
		SM.VertexArrayOffset = FlipUINT(LastOffset);
		LastOffset += FlipUINT(SM.VertexCount) * sizeof(Vec3f);

		SM.UVCount = FlipUINT((uint32_t)Textures.size());
		SM.UVOffset = FlipUINT(LastOffset);
		LastOffset += FlipUINT(SM.UVCount) * sizeof(Vec3f);

		SM.NormalCount = FlipUINT((uint32_t)Normals.size());
		SM.NormalOffset = FlipUINT(LastOffset);
		LastOffset += FlipUINT(SM.NormalCount) * sizeof(Vec3f);

		SM.ColorCount = FlipUINT((uint32_t)Colors.size());
		SM.ColorOffset = FlipUINT(LastOffset);
		LastOffset += FlipUINT(SM.ColorCount) * sizeof(Vec4f);

		const size_t TotalBufferSize = LastOffset;

		// Init our Blob Buffer
		uint8_t* HeaderBlob = new uint8_t[TotalBufferSize];

		std::memcpy(HeaderBlob, &SM, sizeof(StaticMesh));
		// Copy the Data
		// To: The Blob + Little Endian address offset
		// From: The original Vector
		// Count: Same as the LastOffset increment
		std::memcpy(HeaderBlob + FlipUINT(SM.IndexArrayOffset),		Indices.data(),		Indices.size() * sizeof(Vec3i));
		std::memcpy(HeaderBlob + FlipUINT(SM.VertexArrayOffset),		Verts.data(),		Verts.size() * sizeof(Vec3f));
		std::memcpy(HeaderBlob + FlipUINT(SM.UVOffset),				Textures.data(),	Textures.size() * sizeof(Vec3f));
		std::memcpy(HeaderBlob + FlipUINT(SM.NormalOffset),		 	Normals.data(),		Normals.size() * sizeof(Vec3f));
		std::memcpy(HeaderBlob + FlipUINT(SM.ColorOffset),		 		Colors.data(),		Colors.size() * sizeof(Vec4f));

		std::string SubStr = FilePath;
		SubStr.erase(SubStr.length()-3, 3);
		if (g_bUsesOpenGL)
			SubStr += "glsm";
		else
			SubStr += "gcsm";
		ExportMeshToFile(SubStr.c_str(), HeaderBlob, TotalBufferSize);
	}

	return true;
}

void ExportMeshToFile(const char* FilePath, const uint8_t* HeaderBlob, const size_t TotalBufferSize)
{
	std::ofstream OutFile(FilePath, std::ios::binary | std::ios::out);

	if (!OutFile.is_open())
	{
		std::cout << "Error: Failed to open file for writing: " << FilePath << '\n';
		return;
	}

	// Cast to const char* and write to file
	OutFile.write(reinterpret_cast<const char*>(HeaderBlob), TotalBufferSize);

	OutFile.close();
	std::cout << "Exported " << TotalBufferSize << " bytes to " << FilePath << '\n';
}

unordered_set<std::string> supportedFiles = {
	"fbx", "obj"
};

unordered_set<std::string> glFlags = {
	"gl", "opengl", "ogl", "gles"
};

bool IsFileSupportedFormat(const char* Arg)
{
	std::string ArgString(Arg);
	return supportedFiles.contains(ArgString.substr(ArgString.length()-3, 3));
}

bool IsOpenGL(const char* Arg)
{
	std::string ArgString(Arg);

	if (Arg[0] == '-')
		ArgString.erase(0,1);

	return glFlags.contains(ArgString);
}