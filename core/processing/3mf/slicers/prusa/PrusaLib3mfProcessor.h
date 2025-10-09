#ifndef PRUSALIB3MFPROCESSOR_H
#define PRUSALIB3MFPROCESSOR_H

#include "../../BaseLib3mfProcessor.h"
#include "ModelConverter.h"

class PrusaLib3mfProcessor : public BaseLib3mfProcessor {
public:
    PrusaLib3mfProcessor() = default;
    virtual ~PrusaLib3mfProcessor() = default;

    // Implementation of pure virtual methods from base class
    bool setMetaData(double maxStress) override;
    bool setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings, const std::vector<MeshInfo>& meshInfos) override;
    bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) override;
    bool setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh) override;
    bool assembleObjects() override;

    
    // New method that takes extractDir and converter for full implementation
    bool setMetaData(const std::string& extractDir, const ModelConverter& converter,
                    const std::vector<MeshInfo>& meshInfos,
                    const std::vector<StressDensityMapping>& mappings,
                    double maxStress);

private:
    // Prusa-specific helper methods
    std::string setMetaDataForInfillMeshXML(const ModelObjectInfo& objInfo, size_t volumeId, 
                                           const MeshInfo* meshInfo, 
                                           const std::vector<StressDensityMapping>& mappings, 
                                           double maxStress);
    std::string setMetaDataForOutlineMeshXML(const ModelObjectInfo& objInfo, size_t volumeId);
    
    double calculateFillDensity(const MeshInfo& meshInfo, 
                               const std::vector<StressDensityMapping>& mappings, 
                               double maxStress);
};

#endif