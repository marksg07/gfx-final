#ifndef TETMESHPARSER_H
#define TETMESHPARSER_H
#include "ui/SupportCanvas3D.h"
#include "scenegraph/Scene.h"
#include "openglshape.h"
#include "ui/mainwindow.h"
#include "gl/shaders/ShaderAttribLocations.h"
#include "tetgen/tetgen.h"

namespace TetmeshParser
{
    bool parse(std::string filename, tetgenio *out);
    bool parseObjFile(std::string filename, tetgenio *out);
    bool parseMeshFile(std::string filename, tetgenio *out);
}

#endif // TETMESHPARSER_H
