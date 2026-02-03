#include "CuraLib3mfProcessor.h"
#include "../../../../../utils/SettingsManager.h"
#include <iostream>
#include <regex>
#include <map>

bool CuraLib3mfProcessor::setMetaData(double maxStress) {
    std::vector<StressDensityMapping> emptyMappings;
    std::vector<MeshInfo> emptyMeshInfos;
    return setMetaData(maxStress, emptyMappings, emptyMeshInfos);
}

bool CuraLib3mfProcessor::setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings, const std::vector<MeshInfo>& meshInfos) {
    auto meshIterator = model->GetMeshObjects();
    std::regex filePattern(
        R"(^modifierMesh(\d+)\.stl$)"
    );
    for (; meshIterator->MoveNext(); ) {
        Lib3MF::PMeshObject currentMesh = meshIterator->GetCurrentMeshObject();
        auto name = currentMesh->GetName();
        std::smatch match;
        if (std::regex_match(name, match, filePattern)) {
            std::string meshID_str = match[1].str();
            int meshID = std::stoi(meshID_str);
            
            // Find the corresponding MeshInfo by ID
            FileInfo fileInfo;
            fileInfo.id = meshID;
            fileInfo.name = name;
            fileInfo.minStress = 0; // Default values
            fileInfo.maxStress = 0;
            
            // Look up stress values from meshInfos
            for (const auto& meshInfo : meshInfos) {
                if (meshInfo.meshID == meshID) {
                    fileInfo.minStress = meshInfo.stressMin;
                    fileInfo.maxStress = meshInfo.stressMax;
                    break;
                }
            }
            
            setMetaDataForInfillMesh(currentMesh, fileInfo, maxStress, mappings);
        } else {
            setMetaDataForOutlineMesh(currentMesh);
        }
    }
    return true;
}

bool CuraLib3mfProcessor::setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress) {
    std::vector<StressDensityMapping> emptyMappings;
    return setMetaDataForInfillMesh(Mesh, fileInfo, maxStress, emptyMappings);
}

bool CuraLib3mfProcessor::setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) {
    std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
    PMetaDataGroup metadataGroup = Mesh->GetMetaDataGroup();
    double aveStress = (fileInfo.minStress + fileInfo.maxStress) / 2;
    int density = 0;
    // Find the mapping whose range includes aveStress
    for (const auto& mapping : mappings) {
        if (aveStress >= mapping.stressMin && aveStress < mapping.stressMax) {
            density = static_cast<int>(mapping.density);
            break;
        }
    }
    std::string density_str = std::to_string(density);
    metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "False", "xs:boolean", false);
    metadataGroup->AddMetaData(cura_uri, "infill_mesh", "True", "xs:boolean", false);
    metadataGroup->AddMetaData(cura_uri, "infill_sparse_density", density_str, "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "infill_pattern", SettingsManager::instance().infillPattern(), "xs:string", false);
    metadataGroup->AddMetaData(cura_uri, "wall_line_count", "0","xs:integer", false );
    metadataGroup->AddMetaData(cura_uri, "wall_thickness", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "bottom_layers", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "bottom_thickness", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "top_bottom_thickness", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "top_layers", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "top_thickness", "0", "xs:integer", false); 
    return true;
}

bool CuraLib3mfProcessor::setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh){
    std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
    PMetaDataGroup metadataGroup = Mesh->GetMetaDataGroup();
    metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "True", "xs:boolean", false);
    return true;
}

bool CuraLib3mfProcessor::assembleObjects(){
    sTransform identityTransform;
    auto transform = lib3mf_getidentitytransform(&identityTransform);
    auto mergedObject = model->AddComponentsObject();
    auto meshIterator = model->GetMeshObjects();
    int meshCount = meshIterator->Count();
    for (int i = 0; i < meshCount; i++) {
        meshIterator->MoveNext();
        auto currentMesh = meshIterator->GetCurrentMeshObject();
        mergedObject->AddComponent(currentMesh.get(), identityTransform);
    }
    auto metadataGroup = mergedObject->GetMetaDataGroup();
    mergedObject->SetName("Group #1");
    std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
    metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "True", "xs:boolean", false);
    metadataGroup->AddMetaData(cura_uri, "print_order", "1", "xs:integer", false);

    //buildオブジェクトのすべてのメッシュを削除して、mergedオブジェクトを追加
    auto buildItemIterator = model->GetBuildItems();
    for (int i = 0; i < buildItemIterator->Count(); i++) {
        buildItemIterator->MoveNext();
        auto buildItem = buildItemIterator->GetCurrent();
        model->RemoveBuildItem(buildItem);
    }
    
    model->AddBuildItem(mergedObject.get(), identityTransform);
    return true;
}