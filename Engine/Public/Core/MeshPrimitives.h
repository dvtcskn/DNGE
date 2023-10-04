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

#pragma once

#include "Engine/AbstractEngine.h"

namespace MeshPrimitives
{
	inline std::array<FVector2, 4> Create2DPlaneVerticesFromRect(const FBounds2D& Rect)
	{
		std::array<FVector2, 4> Plane;
		Plane[0] = FVector2(Rect.Min.X, Rect.Min.Y);
		Plane[1] = FVector2(Rect.Max.X, Rect.Min.Y);
		Plane[2] = FVector2(Rect.Max.X, Rect.Max.Y);
		Plane[3] = FVector2(Rect.Min.X, Rect.Max.Y);
		return Plane;
	}

	inline std::array<FVector2, 4> Create2DPlaneVerticesFromDimension(const FDimension2D& Dimension)
	{
		return Create2DPlaneVerticesFromRect(FBounds2D(Dimension, FVector2::Zero()));
	}

	inline std::array<FVector2, 6> Create2DHexagonVerticesFromRect(const FBounds2D& Rect)
	{
		const FVector2 center((Rect.Min.X + Rect.Max.X) / 2.0f, (Rect.Min.Y + Rect.Max.Y) / 2.0f);
		const float radius = std::min(Rect.Max.X - center.X, Rect.Max.Y - center.Y);

		std::array<FVector2, 6> Hexagon;
		for (int i = 0; i < 6; ++i)
		{
			const float angle = (60.0f * i - 30.0f) * fPI / 180.0f;
			const FVector2 point(radius * std::cos(angle), radius * std::sin(angle));
			Hexagon[i] = point + center;
		}

		return Hexagon;
	}

	inline std::array<FVector2, 6> Create2DHexagonVerticesFromDimension(const FDimension2D& Dimension)
	{
		return Create2DHexagonVerticesFromRect(FBounds2D(Dimension, FVector2::Zero()));
	}

	inline std::vector<FVector2> GenerateHexagonTextureCoordinate(const float Angle)
	{
		auto RotateCornerPoint = [](FVector2& Edge, float angle)
		{
			FVector2 Center = FVector2(0.5f, 0.5f);
			double theta = (angle * (3.14159265359 / 180));

			float cs = static_cast<float>(cos(theta));
			float sn = static_cast<float>(sin(theta));

			float x = Edge.X - Center.X;
			float y = Edge.Y - Center.Y;

			Edge.X = x * cs - y * sn + Center.X;
			Edge.Y = (x * sn + y * cs + Center.Y)/* * (-1)*/;
		};
		FVector2 P1 = FVector2(static_cast<float>(0.066987), static_cast<float>(0.250000));
		FVector2 P2 = FVector2(static_cast<float>(0.066987), static_cast<float>(0.750000));
		FVector2 P3 = FVector2(static_cast<float>(0.500000), static_cast<float>(1.000000));
		FVector2 P4 = FVector2(static_cast<float>(0.933013), static_cast<float>(0.750000));
		FVector2 P5 = FVector2(static_cast<float>(0.933013), static_cast<float>(0.250000));
		FVector2 P6 = FVector2(static_cast<float>(0.500000), static_cast<float>(0.00000));
		if (Angle != 0.0f)
		{
			RotateCornerPoint(P1, Angle);
			RotateCornerPoint(P2, Angle);
			RotateCornerPoint(P3, Angle);
			RotateCornerPoint(P4, Angle);
			RotateCornerPoint(P5, Angle);
			RotateCornerPoint(P6, Angle);
		}
		return std::vector<FVector2>{ P1, P2, P3, P4, P5, P6 };
	}

	inline std::vector<FVector2> GeneratePlaneTextureCoordinate(const float Angle)
	{
		auto RotateCornerPoint = [](FVector2& Edge, float angle)
		{
			FVector2 Center = FVector2(0.5f, 0.5f);
			double theta = (angle * (3.14159265359 / 180));

			float cs = static_cast<float>(cos(theta));
			float sn = static_cast<float>(sin(theta));

			float x = Edge.X - Center.X;
			float y = Edge.Y - Center.Y;

			Edge.X = x * cs - y * sn + Center.X;
			Edge.Y = (x * sn + y * cs + Center.Y)/* * (-1)*/;
		};
		FVector2 P1 = FVector2(static_cast<float>(0.0), static_cast<float>(0.0));
		FVector2 P2 = FVector2(static_cast<float>(1.0), static_cast<float>(0.0));
		FVector2 P3 = FVector2(static_cast<float>(1.0), static_cast<float>(1.0));
		FVector2 P4 = FVector2(static_cast<float>(0.0), static_cast<float>(1.0));
		if (Angle != 0.0f)
		{
			RotateCornerPoint(P1, Angle);
			RotateCornerPoint(P2, Angle);
			RotateCornerPoint(P3, Angle);
			RotateCornerPoint(P4, Angle);
		}
		return std::vector<FVector2>{ P1, P2, P3, P4 };
	}

	// hexagon_indices: vector of int to hold the generated indices
	inline std::vector<std::uint32_t> GenerateHexagonIndices(const std::uint32_t PrimitiveSize)
	{
		std::vector<std::uint32_t> Indices;
		for (std::uint32_t i = 0; i < PrimitiveSize; i++)
		{
			Indices.push_back(0 + (6 * i));
			Indices.push_back(5 + (6 * i));
			Indices.push_back(4 + (6 * i));

			Indices.push_back(0 + (6 * i));
			Indices.push_back(4 + (6 * i));
			Indices.push_back(3 + (6 * i));

			Indices.push_back(0 + (6 * i));
			Indices.push_back(3 + (6 * i));
			Indices.push_back(2 + (6 * i));

			Indices.push_back(2 + (6 * i));
			Indices.push_back(1 + (6 * i));
			Indices.push_back(0 + (6 * i));
		}
		return Indices;
	}

	inline std::vector<std::uint32_t> GeneratePlaneIndices(const std::uint32_t PrimitiveSize)
	{
		std::vector<std::uint32_t> Indices;
		for (std::uint32_t i = 0; i < PrimitiveSize; i++)
		{
			Indices.push_back(3 + (4 * i));
			Indices.push_back(1 + (4 * i));
			Indices.push_back(0 + (4 * i));

			Indices.push_back(3 + (4 * i));
			Indices.push_back(2 + (4 * i));
			Indices.push_back(1 + (4 * i));
		}
		return Indices;
	}

	inline std::vector<std::array<FVector2, 4>> GenerateSquareGrid(std::size_t GridX, std::size_t GridY, FDimension2D CellDimension)
	{
		const std::size_t TotalGridSize = (GridX * GridY);

		const float cellScale = 1.0f;

		const FVector2 gridLoc = FVector2::Zero();
		const float GridOffsetX = 0.0f;
		const float GridOffsetY = 0.0f;

		const auto Vertices = MeshPrimitives::Create2DPlaneVerticesFromDimension(CellDimension);
		const FVector2 NodeBoundMin(Vertices[0].X, Vertices[0].Y);
		const FVector2 NodeBoundMax(Vertices[2].X, Vertices[2].Y);

		const float boundX = NodeBoundMax.X - NodeBoundMin.X + GridOffsetX;
		const float boundY = NodeBoundMax.Y - NodeBoundMin.Y - GridOffsetY;

		std::vector<std::array<FVector2, 4>> Tiles;

		for (std::size_t i = 0; i <= TotalGridSize; i++)
		{
			const FVector2 NodeLocRef = FVector2(((i % GridY) * boundY), ((i / GridY) * boundX));

			Tiles.insert(Tiles.end(), MeshPrimitives::Create2DPlaneVerticesFromRect(FBounds2D(CellDimension, gridLoc - Negate(NodeLocRef * cellScale))));
		}
		return Tiles;
	}

	inline std::vector<std::array<FVector2, 6>> GenerateHexagonGrid(std::size_t GridX, std::size_t GridY, FDimension2D Plane2DDimension)
	{
		const std::size_t TotalGridSize = (GridX * GridY);

		const float cellScale = 1.0f;

		const FVector2 gridLoc = FVector2::Zero();
		const float GridOffsetX = 0.0f;
		const float GridOffsetY = 0.0f;

		const auto Vertices = MeshPrimitives::Create2DHexagonVerticesFromDimension(Plane2DDimension);
		const FVector2 NodeBoundMin(0.0f, 0.0f);
		// HexRatio 2/3 (height-to-width ratio)
		//const FVector2 NodeBoundMax(Vertices[0].X * 2, Plane2DDimension.Height + Vertices[0].Y);
		const FVector2 NodeBoundMax(Vertices[0].X * 2, std::abs(Vertices[0].Y) * 3);

		const float boundX = NodeBoundMax.X - NodeBoundMin.X + GridOffsetX;
		const float boundY = NodeBoundMax.Y - NodeBoundMin.Y - GridOffsetY;

		std::vector<std::array<FVector2, 6>> Tiles;

		for (std::size_t i = 0; i <= TotalGridSize; i++)
		{
			const FVector2 NodeLocRef = FVector2(((((i / GridX) % 2) * (boundX / 2.0f)) + ((i % GridX) * boundX)), ((i / GridX) * (boundY * 1.0f)));

			Tiles.insert(Tiles.end(), MeshPrimitives::Create2DHexagonVerticesFromRect(FBounds2D(Plane2DDimension, gridLoc - Negate(NodeLocRef * cellScale))));
		}
		return Tiles;
	}

	inline sMeshData CreateBox()
	{
		const float Scale = 0.05f;
		sMeshData MeshData;
		{
			MeshData.Indices.push_back(3);
			MeshData.Indices.push_back(1);
			MeshData.Indices.push_back(0);
			MeshData.Indices.push_back(3);
			MeshData.Indices.push_back(2);
			MeshData.Indices.push_back(1);
			MeshData.Indices.push_back(7);
			MeshData.Indices.push_back(5);
			MeshData.Indices.push_back(4);
			MeshData.Indices.push_back(7);
			MeshData.Indices.push_back(6);
			MeshData.Indices.push_back(5);
			MeshData.Indices.push_back(11);
			MeshData.Indices.push_back(9);
			MeshData.Indices.push_back(8);
			MeshData.Indices.push_back(11);
			MeshData.Indices.push_back(10);
			MeshData.Indices.push_back(9);
			MeshData.Indices.push_back(15);
			MeshData.Indices.push_back(13);
			MeshData.Indices.push_back(12);
			MeshData.Indices.push_back(15);
			MeshData.Indices.push_back(14);
			MeshData.Indices.push_back(13);
			MeshData.Indices.push_back(19);
			MeshData.Indices.push_back(17);
			MeshData.Indices.push_back(16);
			MeshData.Indices.push_back(19);
			MeshData.Indices.push_back(18);
			MeshData.Indices.push_back(17);
			MeshData.Indices.push_back(23);
			MeshData.Indices.push_back(21);
			MeshData.Indices.push_back(20);
			MeshData.Indices.push_back(23);
			MeshData.Indices.push_back(22);
			MeshData.Indices.push_back(21);
			sVertexBufferEntry VBE;
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(0.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(-1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(0.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
			VBE.position = { static_cast<float>(1.000000) * Scale, static_cast<float>(1.000000) * Scale, static_cast<float>(-1.000000) * Scale };
			VBE.texCoord = { static_cast<float>(1.000000), static_cast<float>(1.000000) };
			MeshData.Vertices.push_back(VBE);
		}
		MeshData.DrawParameters.IndexCountPerInstance = (std::uint32_t)MeshData.Indices.size();
		return MeshData;
	}
}
