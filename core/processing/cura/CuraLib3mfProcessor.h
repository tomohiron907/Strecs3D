#ifndef CURALIB3MFPROCESSOR_H
#define CURALIB3MFPROCESSOR_H

#include "../BaseLib3mfProcessor.h"

class CuraLib3mfProcessor : public BaseLib3mfProcessor {
public:
    CuraLib3mfProcessor() = default;
    virtual ~CuraLib3mfProcessor() = default;

    // Implementation of pure virtual methods from base class
    bool setMetaData(double maxStress) override;
    bool setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings) override;
    bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) override;
    bool setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh) override;
    bool assembleObjects() override;

private:
    // Cura-specific helper methods
    bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress);
};

#endif