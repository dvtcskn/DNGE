/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "Core/Math/CoreMath.h"
#include "Engine/AbstractEngine.h"

class OBJImporter
{
public:
	struct OBJ
	{
		struct MTL
		{
			std::string Name;
			float Ns;
			FVector Ka;
			FVector Kd = FVector::One();
			FVector Ks;
			FVector Ke;
			float Ni;
			float D;
			int illum;

			std::map<std::string, std::string> Maps;
		};

		struct ObjMesh
		{
			std::vector<FVector> Positions;
			std::vector<FVector> Normals;
			std::vector<FVector2> TextureCoords;

			std::vector<std::uint32_t> Indices;
		};

		struct Face
		{
			std::uint32_t MeshIndex;
			std::uint32_t Position;
			std::uint32_t TextureCoord;
			std::uint32_t Normals;
		};

		struct OBJPart
		{
			std::string PartName;
			ObjMesh Mesh;
			std::vector<FVector> Verts;
			std::vector<FVector> Normals;
			std::vector<FVector2> TextureCoords;
			std::vector<Face> Faces;
			std::vector<std::string> MaterialNames;
			std::map<std::string, std::vector<Face>> FacesByMaterial;
			int s; // ???
		};

		std::string Name;
		ObjMesh Mesh;
		std::string MTLibName;
		std::vector<FVector> Verts;
		std::vector<FVector> Normals;
		std::vector<FVector2> TextureCoords;
		std::vector<Face> Faces;
		std::vector<std::string> MaterialNames;
		std::vector<OBJPart> Parts;
		std::vector<MTL> MTLs;
	};
public:
	OBJImporter();
	~OBJImporter();

	bool Import(const std::string& path, bool bFlipTextCoordY = false);

	OBJ obj;

private:

};
