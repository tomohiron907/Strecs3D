#ifndef BAMBULIB3MFPROCESSOR_H
#define BAMBULIB3MFPROCESSOR_H

#include "../../BaseLib3mfProcessor.h"
#include "../../../../../utils/BambuProjectSettings.h"
#include <optional>

class BambuLib3mfProcessor : public BaseLib3mfProcessor {
private:
    xmlconverter::Config config;
    xmlconverter::Object object;

public:
    BambuLib3mfProcessor() = default;
    virtual ~BambuLib3mfProcessor() = default;

    // Implementation of pure virtual methods from base class
    bool setMetaData(double maxStress) override;
    bool setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings, const std::vector<MeshInfo>& meshInfos) override;
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
    // Bambu-specific constants
    static constexpr float MODIFIER_MESH_Z_OFFSET = -0.4f;

    std::optional<BambuProjectSettings> projectSettings;
    // 元形状をプレート中央の床面に置くための平行移動量
    sTransform buildTransform;
    
    // Bambu-specific helper methods
    bool setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress);
    bool setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings);
    bool setMetaDataForOutlineMeshBambu(Lib3MF::PMeshObject Mesh);
    
    // Offset modifier mesh vertices in Z direction
    bool offsetModifierMeshZ(Lib3MF::PMeshObject mesh, float zOffset);

    // Compute buildTransform from the bounding box of the outline mesh
    void updateBuildTransform(Lib3MF::PMeshObject outlineMesh);
    std::string buildTranslationString() const;
    bool exportProjectSettings(const std::string& outputDir);
};

#endif