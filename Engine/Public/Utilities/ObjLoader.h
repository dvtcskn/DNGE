
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Core/Math/CoreMath.h"
#include "Engine/AbstractEngine.h"

class ObjLoader 
{
    sBaseClassBody(sClassConstructor, ObjLoader)
public:
    ObjLoader(const std::string& filename)
    {
        Load(filename);
    }

    bool Load(const std::string& filename) 
    {
        
    }

    sMeshData MeshData;
};
