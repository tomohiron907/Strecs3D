#ifndef BASELIB3MFPROCESSOR_H
#define BASELIB3MFPROCESSOR_H

#include "lib3mf_implicit.hpp"
using namespace Lib3MF;

#include "../../../utils/xmlConverter.h"
#include <vector>
#include "../../types/StressDensityMapping.h"

struct FileInfo {
    int id;
    std::string name;
    double minStress;
    double maxStress;
};

class BaseLib3mfProcessor {
protected:
    PWrapper wrapper = CWrapper::loadLibrary();
    PModel model = wrapper->CreateModel();
    PReader reader = model->QueryReader("stl");

public:
    virtual ~BaseLib3mfProcessor() = default;

    // Common methods implemented in the base class
    bool getMeshes();
    bool setStl(const std::string stlFileName);
    bool save3mf(const std::string outputFilename);

    // Pure virtual methods that must be implemented by derived classes
    virtual bool setMetaData(double maxStress) = 0;
    virtual bool setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings) = 0;
    virtual bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) = 0;
    virtual bool setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh) = 0;
    virtual bool assembleObjects() = 0;
};

#endif