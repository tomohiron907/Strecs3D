#ifndef BAMBULIB3MFPROCESSOR_H
#define BAMBULIB3MFPROCESSOR_H

#include "../BaseLib3mfProcessor.h"

class BambuLib3mfProcessor : public BaseLib3mfProcessor {
private:
    xmlconverter::Config config;
    xmlconverter::Object object;

public:
    BambuLib3mfProcessor() = default;
    virtual ~BambuLib3mfProcessor() = default;

    // Implementation of pure virtual methods from base class
    bool setMetaData(double maxStress) override;
    bool setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings) override;
    bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) override;
    bool setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh) override;
    bool assembleObjects() override;

    // Bambu-specific methods
    bool setObjectDataBambu(int meshCount);
    bool setPlateDataBambu(int meshCount);
    bool setAssembleDataBambu(int meshCount);
    bool setupBuildObjects();
    bool exportConfig();

private:
    // Bambu-specific helper methods
    bool setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress);
    bool setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings);
    bool setMetaDataForOutlineMeshBambu(Lib3MF::PMeshObject Mesh);
};

#endif