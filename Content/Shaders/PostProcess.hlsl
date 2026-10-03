
struct FullScreenTriangleVSOut
{
    float2 texCoord : texCoord;
    float4 positionViewport : SV_Position;
    //float4 positionClip : positionClip;
};

FullScreenTriangleVSOut FullScreenTriangleVS(uint vertexID : SV_VertexID)
{
    FullScreenTriangleVSOut output;

	// Parametrically work out vertex location for full screen triangle
    float2 grid = float2((vertexID << 1) & 2, vertexID & 2);
    float4 /*output.*/positionClip = float4(grid * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);
    output.positionViewport = /*output.*/positionClip;
    output.texCoord = grid;

    return output;
}

struct VertexOutput
{
    // Other output data (e.g. texture coordinates, normals, etc.)
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

// Vertex input structure
struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

// Full-screen quad vertices
//
//  (-1, 1) (1, 1)
//   +-------+
//   |       |
//   |       |
//   +-------+
// (-1, -1) (1, -1)

static const VertexInput fullScreenQuadVertices[6] =
{
    { float4(-1.0, 1.0, 0.0, 1.0), float2(0.0, 0.0) },
    { float4(-1.0, -1.0, 0.0, 1.0), float2(0.0, 1.0) },
    { float4(1.0, 1.0, 0.0, 1.0), float2(1.0, 0.0) },
    { float4(1.0, 1.0, 0.0, 1.0), float2(1.0, 0.0) },
    { float4(-1.0, -1.0, 0.0, 1.0), float2(0.0, 1.0) },
    { float4(1.0, -1.0, 0.0, 1.0), float2(1.0, 1.0) },
};

// Vertex shader
VertexOutput mainFSQ(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    output.position = fullScreenQuadVertices[vertexID].position;
    output.texCoord = fullScreenQuadVertices[vertexID].texCoord;
    return output;
}
//// Vertex input structure
//struct VertexInput
//{
//    float4 position : POSITION;
//};

//// Vertex shader
//VertexOutput mainFST(VertexInput input)
//{
//    VertexOutput output;
//    output.position = input.position;
//    return output;
//}

//// Full-screen triangle vertices
////
////  (0, 1) (1, 0) (-1, 0)
////   +-------+
////   |       |
////   +-------+
//// (0, -1)

//const VertexInput fullScreenTriangleVertices[3] =
//{
//    { float4(0.0, 1.0, 0.0, 1.0) },
//    { float4(1.0, 0.0, 0.0, 1.0) },
//    { float4(-1.0, 0.0, 0.0, 1.0) },
//};
