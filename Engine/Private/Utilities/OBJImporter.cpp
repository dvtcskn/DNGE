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

#include "pch.h"
#include "Utilities/OBJImporter.h"
#include "Utilities/FileManager.h"
#include "Core/Archive.h"

OBJImporter::OBJImporter()
{
}

OBJImporter::~OBJImporter()
{
}

bool OBJImporter::Import(const std::string& path)
{
	std::vector<std::string> Lines;

	std::uint32_t PartSize = 0;
	if (!path.empty())
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			file.seekg(0, std::ios::beg);
			std::string line;
			while (std::getline(file, line))
			{
				Lines.push_back(line);
				if (line.find("o ") != -1)
				{
					PartSize++;
				}
			}
		}
	}

	if (PartSize == 0)
		return false;

	std::string MtLib;
	for (const auto& Line : Lines)
	{
		if (Line.find("mtllib ") != -1)
		{
			std::istringstream sin(Line.substr(Line.find(" ") + 1));
			sin >> MtLib;
			break;
		}
	}

	obj.Parts.resize(PartSize);

	std::string str = path;
	std::size_t found = str.find_last_of("//\\");
	obj.Name = std::string(str.begin() + found + 1, str.end());

	obj.MTLibName = MtLib;

	std::string CurrentMaterialName = "";
	std::int32_t PartCounter = -1;
	for (const auto& Line : Lines)
	{
		if (Line.empty())
			continue;

		std::size_t VarOffset = Line.find(" ");
		std::string Var = std::string(Line.begin(), Line.begin() + VarOffset);

		if (Var == "o")
		{
			PartCounter++;
			std::istringstream sin(Line.substr(Line.find(" ") + 1));
			sin >> obj.Parts[PartCounter].PartName;
		}
		else if (Var == "v")
		{
			float X, Y, Z = 0.0f;
			std::size_t Offset = 0;
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> X;
			}
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> Y;
			}
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> Z;
			}
			obj.Parts[PartCounter].Verts.push_back(FVector(X, Y, Z));
			obj.Verts.push_back(FVector(X, Y, Z));
		}
		else if (Var == "vn")
		{
			float X, Y, Z = 0.0f;
			std::size_t Offset = 0;
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> X;
			}
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> Y;
			}
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> Z;
			}
			obj.Parts[PartCounter].Normals.push_back(FVector(X, Y, Z));
			obj.Normals.push_back(FVector(X, Y, Z));
		}
		else if (Var == "vt")
		{
			float X, Y = 0.0f;
			std::size_t Offset = 0;
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> X;
			}
			{
				Offset = Line.find(" ", Offset) + 1;
				std::istringstream sin(Line.substr(Offset));
				sin >> Y;
			}
			obj.Parts[PartCounter].TextureCoords.push_back(FVector2(X, Y));
			obj.TextureCoords.push_back(FVector2(X, Y));
		}
		else if (Var == "s")
		{
			std::istringstream sin(Line.substr(Line.find(" ") + 1));
			sin >> obj.Parts[PartCounter].s;
		}
		else if (Var == "usemtl")
		{
			std::istringstream sin(Line.substr(Line.find(" ") + 1));
			std::string Mat;
			sin >> Mat;
			obj.MaterialNames.push_back(Mat);
			obj.Parts[PartCounter].MaterialNames.push_back(Mat);
			CurrentMaterialName = Mat;
		}
		else if (Var == "f")
		{
			std::vector<std::string> Indexes;
			std::string IndexLine;
			for (const char& Word : Line)
			{
				if (Word == ' ' || Word == '/' || Word == 'f')
				{
					if (!IndexLine.empty())
					{
						Indexes.push_back(IndexLine);
						IndexLine.clear();
					}
					continue;
				}
				IndexLine.push_back(Word);
			}

			if (!IndexLine.empty())
			{
				Indexes.push_back(IndexLine);
				IndexLine.clear();
			}

			std::vector<OBJ::Face> Faces;
			std::size_t idx = 0;
			OBJ::Face face;
			const bool MinusOne = true;
			for (const auto& idxSTR : Indexes)
			{
				std::uint32_t FaceIndex = 0;
				std::istringstream sin(idxSTR);
				sin >> FaceIndex;
				if (idx == 0)
					face.Position = MinusOne ? FaceIndex - 1 : FaceIndex;
				else if (idx == 1)
					face.TextureCoord = MinusOne ? FaceIndex - 1 : FaceIndex;
				else if (idx == 2)
					face.Normals = MinusOne ? FaceIndex - 1 : FaceIndex;

				if (idx == 2)
				{
					Faces.push_back(face);
					idx = 0;

					obj.Faces.push_back(face);
					obj.Parts[PartCounter].Faces.push_back(face);
					obj.Parts[PartCounter].FacesByMaterial[CurrentMaterialName].push_back(face);

					face = OBJ::Face();
					continue;
				}
				idx++;
			}

			//for (const auto& String : Indexes)
			//{
			//	std::uint32_t Index = 0;
			//	std::istringstream sin(String);
			//	sin >> Index;
			//	//obj.Faces.push_back(Index - 1);
			//	//obj.Parts[PartCounter].Faces.push_back(Index - 1);
			//	//obj.Parts[PartCounter].FacesByMaterial[CurrentMaterialName].push_back(Index - 1);
			//}
		}
	}

	if (!MtLib.empty())
	{
		std::vector<std::string> MtLib_Lines;
		std::uint32_t MtLib_Size = 0;

		std::string MtLibPath = "";
		{
			std::string str = path;
			std::size_t found = str.find_last_of("//\\");
			MtLibPath = std::string(str.begin(), str.begin() + found + 1);
			MtLibPath.append(MtLib);
		}

		if (!MtLibPath.empty())
		{
			std::ifstream file(MtLibPath, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				file.seekg(0, std::ios::beg);
				std::string line;
				while (std::getline(file, line))
				{
					MtLib_Lines.push_back(line);
					if (line.find("newmtl ") != -1)
					{
						MtLib_Size++;
					}
				}
			}
		}

		if (MtLib_Size != 0)
		{
			obj.MTLs.resize(MtLib_Size);

			std::int32_t MatCounter = -1;
			for (const auto& Line : MtLib_Lines)
			{
				if (Line.empty())
					continue;

				std::size_t VarOffset = Line.find(" ");
				std::string Var = std::string(Line.begin(), Line.begin() + VarOffset);

				if (Var == "newmtl")
				{
					MatCounter++;
					std::istringstream sin(Line.substr(Line.find(" ") + 1));
					sin >> obj.MTLs[MatCounter].Name;
				}
				else if (Var == "Ns")
				{
					std::istringstream sin(Line.substr(Line.find(" ") + 1));
					sin >> obj.MTLs[MatCounter].Ns;
				}
				else if (Var == "Ka")
				{
					float X, Y, Z = 0.0f;
					std::size_t Offset = 0;
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> X;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Y;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Z;
					}
					obj.MTLs[PartCounter].Ka = (FVector(X, Y, Z));
				}
				else if (Var == "Kd")
				{
					float X, Y, Z = 0.0f;
					std::size_t Offset = 0;
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> X;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Y;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Z;
					}
					obj.MTLs[PartCounter].Kd = (FVector(X, Y, Z));
				}
				else if (Var == "Ks")
				{
					float X, Y, Z = 0.0f;
					std::size_t Offset = 0;
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> X;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Y;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Z;
					}
					obj.MTLs[PartCounter].Ks = (FVector(X, Y, Z));
				}
				else if (Var == "Ke")
				{
					float X, Y, Z = 0.0f;
					std::size_t Offset = 0;
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> X;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Y;
					}
					{
						Offset = Line.find(" ", Offset) + 1;
						std::istringstream sin(Line.substr(Offset));
						sin >> Z;
					}
					obj.MTLs[PartCounter].Ke = (FVector(X, Y, Z));
				}
				else if (Var == "Ni")
				{
					std::istringstream sin(Line.substr(Line.find(" ") + 1));
					sin >> obj.MTLs[MatCounter].Ni;
				}
				else if (Var == "d")
				{
					std::istringstream sin(Line.substr(Line.find(" ") + 1));
					sin >> obj.MTLs[MatCounter].D;
				}
				else if (Var == "illum")
				{
					std::istringstream sin(Line.substr(Line.find(" ") + 1));
					sin >> obj.MTLs[MatCounter].illum;
				}
				else if (Var.find("map_") != -1)
				{
					std::size_t Offset = 0;
					Offset = Line.find(" ", Offset) + 1;
					std::string MapName = std::string(Line.begin(), Line.begin() + Offset);
					std::string ValueName = "";
					std::istringstream sin(Line.substr(Line.find(" ") + 1));
					sin >> ValueName;
					obj.MTLs[MatCounter].Maps.insert({ MapName, ValueName });
				}
			}
		}		
	}

	for (const auto& Face : obj.Faces)
	{
		obj.Mesh.Positions.push_back(obj.Verts[Face.Position]);
		obj.Mesh.TextureCoords.push_back(obj.TextureCoords[Face.TextureCoord]);
		obj.Mesh.Normals.push_back(obj.Normals[Face.Normals]);
	}

	for (auto& Part : obj.Parts)
	{
		for (const auto& Face : Part.Faces)
		{
			Part.Mesh.Positions.push_back(Part.Verts[Face.Position]);
			Part.Mesh.TextureCoords.push_back(Part.TextureCoords[Face.TextureCoord]);
			Part.Mesh.Normals.push_back(Part.Normals[Face.Normals]);
		}
	}

	for (int i = 0; i < obj.Mesh.Positions.size(); i++)
	{
		//obj.Mesh.Indices.push_back(obj.Faces[i].Position);
		obj.Mesh.Indices.push_back(i);
	}

	return true;
}
