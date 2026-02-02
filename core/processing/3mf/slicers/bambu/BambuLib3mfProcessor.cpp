#include "BambuLib3mfProcessor.h"
#include "../../../../../utils/tempPathUtility.h"
#include <iostream>
#include <regex>
#include <map>
#include <filesystem>

bool BambuLib3mfProcessor::setMetaData(double maxStress) {
    std::vector<StressDensityMapping> emptyMappings;
    std::vector<MeshInfo> emptyMeshInfos;
    return setMetaData(maxStress, emptyMappings, emptyMeshInfos);
}

bool BambuLib3mfProcessor::setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings, const std::vector<MeshInfo>& meshInfos){
    auto meshIterator = model->GetMeshObjects();
    std::regex filePattern(
        R"(^modifierMesh(\d+)\.stl$)"
    );
    for (; meshIterator->MoveNext(); ) {
        Lib3MF::PMeshObject currentMesh = meshIterator->GetCurrentMeshObject();
        auto name = currentMesh->GetName();
        currentMesh->SetType(Lib3MF::eObjectType::Other);

        std::smatch match;
        if (std::regex_match(name, match, filePattern)) {
            std::string meshID_str = match[1].str();
            int meshID = std::stoi(meshID_str);
            
            // モディファイアメッシュのZ座標をオフセット（Bambuモード専用）
            offsetModifierMeshZ(currentMesh, MODIFIER_MESH_Z_OFFSET);
            
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
        }
        else{
            setMetaDataForOutlineMesh(currentMesh);
        }
    }
    setObjectDataBambu(meshIterator->Count());
    setPlateDataBambu(meshIterator->Count());
    setAssembleDataBambu(meshIterator->Count());
    setupBuildObjects();
    exportConfig();
    return true;
}

bool BambuLib3mfProcessor::setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) {
    return setMetaDataForInfillMeshBambu(Mesh, fileInfo, maxStress, mappings);
}

bool BambuLib3mfProcessor::setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh) {
    return setMetaDataForOutlineMeshBambu(Mesh);
}

bool BambuLib3mfProcessor::assembleObjects() {
    return setupBuildObjects();
}

bool BambuLib3mfProcessor::setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress) {
    std::vector<StressDensityMapping> emptyMappings;
    return setMetaDataForInfillMeshBambu(Mesh, fileInfo, maxStress, emptyMappings);
}

bool BambuLib3mfProcessor::setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings){
    xmlconverter::Part part;
    double aveStress = (fileInfo.minStress + fileInfo.maxStress) / 2;
    int density = 0;
    for (const auto& mapping : mappings) {
        if (aveStress >= mapping.stressMin && aveStress < mapping.stressMax) {
            density = static_cast<int>(mapping.density);
            break;
        }
    }
    std::string density_str = std::to_string(density);
    part.id = fileInfo.id+1;
    part.subtype = "modifier_part";
    part.metadata.push_back({"name", fileInfo.name});
    part.metadata.push_back({"matrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1"});
    part.metadata.push_back({"source_file", fileInfo.name});
    part.metadata.push_back({"source_object_id", "0"});
    part.metadata.push_back({"source_volume_id", "0"});
    part.metadata.push_back({"source_offset_x", "0"});
    part.metadata.push_back({"source_offset_y", "0"});
    part.metadata.push_back({"source_offset_z", "0"});
    part.metadata.push_back({"minimum_sparse_infill_area", "0"});
    part.metadata.push_back({"sparse_infill_anchor", "5"});
    part.metadata.push_back({"sparse_infill_anchor_max", "5"});
    part.metadata.push_back({"sparse_infill_density", density_str});
    part.mesh_stat = {0, 0, 0, 0, 0};
    object.parts.push_back(part);
    return true;
}

bool BambuLib3mfProcessor::setMetaDataForOutlineMeshBambu(Lib3MF::PMeshObject Mesh){
    xmlconverter::Part part;
    part.id = Mesh->GetResourceID();
    part.subtype = "normal_part";
    part.metadata.push_back({"name", Mesh->GetName()});
    part.metadata.push_back({"matrix", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1"});
    part.metadata.push_back({"source_file", Mesh->GetName()});
    part.metadata.push_back({"source_object_id", "0"});
    part.metadata.push_back({"source_volume_id", "0"});
    part.metadata.push_back({"source_offset_x", "0"});
    part.metadata.push_back({"source_offset_y", "0"});
    part.metadata.push_back({"source_offset_z", "0"});
    part.metadata.push_back({"sparse_infill_density", "0"});
    part.mesh_stat = {0, 0, 0, 0, 0};

    object.parts.push_back(part);
    return true;
}

bool BambuLib3mfProcessor::setObjectDataBambu(int meshCount){
    object.id = meshCount+1;
    object.metadata.push_back({"name", "Group #1"});
    object.metadata.push_back({"extruder", "1"});
    config.objects.push_back(object);
    return true;
}

bool BambuLib3mfProcessor::setPlateDataBambu(int meshCount){
    xmlconverter::Plate plate;
    plate.metadata.push_back({"plater_id", "1"});
    plate.metadata.push_back({"plater_name", ""});
    plate.metadata.push_back({"locked", "false"});
    plate.metadata.push_back({"thumbnail_file", "Metadata/plate_1.png"});
    plate.metadata.push_back({"thumbnail_no_light_file", "Metadata/plate_no_light_1.png"});
    plate.metadata.push_back({"top_file", "Metadata/top_1.png"});
    plate.metadata.push_back({"pick_file", "Metadata/pick_1.png"});

    // plate 内の model_instance 要素の作成
    plate.model_instance.metadata.push_back({"object_id", std::to_string(meshCount+1)});
    plate.model_instance.metadata.push_back({"instance_id", "0"});
    plate.model_instance.metadata.push_back({"identify_id", "92"});
    config.plates.push_back(plate);
    return true;
}

bool BambuLib3mfProcessor::setAssembleDataBambu(int meshCount){
    xmlconverter::AssembleItem item;
    item.object_id = meshCount+1;
    item.instance_id = 0;
    item.transform = "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";
    item.offset = "0 0 0";
    config.assemble.items.push_back(item);
    return true;
}

bool BambuLib3mfProcessor::setupBuildObjects(){
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
    mergedObject->SetName("Group #1");

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

bool BambuLib3mfProcessor::exportConfig(){
    // 出力先ディレクトリを指定
    const std::string outputDir = TempPathUtility::getTempSubDirPath("3mf/Metadata").string();
    // ディレクトリが存在しなければ作成する
    std::filesystem::create_directories(outputDir);

    // 出力ファイル名を設定
    std::string outputFilename = outputDir + "/model_settings.config";

    // XMLファイル "config.xml" に書き出し
    if (xmlconverter::writeConfigToFile(config, outputFilename)) {
    } else {
        std::cerr << "XMLの書き出しに失敗しました。" << std::endl;
    }
    return true;
}

bool BambuLib3mfProcessor::offsetModifierMeshZ(Lib3MF::PMeshObject mesh, float zOffset) {
    Lib3MF_uint32 vertexCount = mesh->GetVertexCount();
    for (Lib3MF_uint32 i = 0; i < vertexCount; ++i) {
        sPosition pos = mesh->GetVertex(i);
        pos.m_Coordinates[2] += zOffset;  // Z座標をオフセット
        mesh->SetVertex(i, pos);
    }
    return true;
}