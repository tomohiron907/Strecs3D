#include "CuraLib3mfProcessor.h"
#include <iostream>
#include <regex>
#include <map>

bool CuraLib3mfProcessor::setMetaData(double maxStress) {
    std::vector<StressDensityMapping> emptyMappings;
    return setMetaData(maxStress, emptyMappings);
}

bool CuraLib3mfProcessor::setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings) {
    auto meshIterator = model->GetMeshObjects();
    std::regex filePattern(
        R"(^dividedMesh(\d+)_(\d+(?:\.\d+)?)_(\d+(?:\.\d+)?)\.stl$)"
    );
    for (; meshIterator->MoveNext(); ) {
        Lib3MF::PMeshObject currentMesh = meshIterator->GetCurrentMeshObject();
        auto name = currentMesh->GetName();
        std::map<std::string, FileInfo> fileInfoMap;
        std::smatch match;
        if (std::regex_match(name, match, filePattern)) {
            std::string id_str = match[1].str();
            std::string minStress_str = match[2].str();
            std::string maxStress_str = match[3].str();
            FileInfo fileInfo;
            fileInfo.id = std::stoi(id_str);
            fileInfo.name = name;
            fileInfo.minStress = std::stod(minStress_str);
            fileInfo.maxStress = std::stod(maxStress_str);
            fileInfoMap[name] = fileInfo;
            setMetaDataForInfillMesh(currentMesh, fileInfoMap[name], maxStress, mappings);
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