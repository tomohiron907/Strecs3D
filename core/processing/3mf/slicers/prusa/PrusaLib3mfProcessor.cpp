#include "PrusaLib3mfProcessor.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

bool PrusaLib3mfProcessor::setMetaData(double maxStress) {
    // TODO: Implement Prusa-specific metadata setting
    return true;
}

bool PrusaLib3mfProcessor::setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings, const std::vector<MeshInfo>& meshInfos) {
    // TODO: Implement Prusa-specific metadata setting with mappings and mesh info
    return true;
}

bool PrusaLib3mfProcessor::setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings) {
    // TODO: Implement Prusa-specific infill mesh metadata setting
    return true;
}

bool PrusaLib3mfProcessor::setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh) {
    // TODO: Implement Prusa-specific outline mesh metadata setting
    return true;
}

bool PrusaLib3mfProcessor::assembleObjects() {
    // TODO: Implement Prusa-specific object assembly
    return true;
}

bool PrusaLib3mfProcessor::setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress) {
    // TODO: Implement Prusa-specific infill mesh metadata setting (simple version)
    return true;
}

double PrusaLib3mfProcessor::calculateFillDensity(const MeshInfo& meshInfo, 
                                                const std::vector<StressDensityMapping>& mappings, 
                                                double maxStress) {
    // BambuLib3mfProcessorと同じ方法で平均応力値を計算
    double aveStress = (meshInfo.stressMin + meshInfo.stressMax) / 2.0;
    
    for (const auto& mapping : mappings) {
        // BambuLib3mfProcessorと同じ条件でマッチング（< ではなく <= に修正）
        if (aveStress >= mapping.stressMin && aveStress <= mapping.stressMax) {
            return mapping.density;
        }
    }
    
    // デフォルト値として20%を返す（0.2 = 20%）
    return 0.2;
}

std::string PrusaLib3mfProcessor::generateInfillVolumeXML(const ModelObjectInfo& objInfo, size_t volumeId, 
                                                         const MeshInfo* meshInfo, 
                                                         const std::vector<StressDensityMapping>& mappings, 
                                                         double maxStress) {
    std::ostringstream xml;
    
    xml << "  <volume firstid=\"" << objInfo.start_triangle_index 
        << "\" lastid=\"" << objInfo.end_triangle_index << "\">\n";
    
    xml << "   <metadata type=\"volume\" key=\"name\" value=\"" << objInfo.name << "\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"modifier\" value=\"1\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"volume_type\" value=\"ParameterModifier\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"matrix\" value=\"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_file\" value=\"" << objInfo.name << "\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_object_id\" value=\"0\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_volume_id\" value=\"" << volumeId << "\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_offset_x\" value=\"0\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_offset_y\" value=\"0\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_offset_z\" value=\"0\"/>\n";
    
    // fill_density情報を追加
    if (meshInfo) {
        double fillDensity = calculateFillDensity(*meshInfo, mappings, maxStress);
        int fillPercent = static_cast<int>(fillDensity);
        xml << "   <metadata type=\"volume\" key=\"fill_density\" value=\"" 
            << fillPercent << "%\"/>\n";
    }
    
    xml << "   <mesh edges_fixed=\"0\" degenerate_facets=\"0\" facets_removed=\"0\" facets_reversed=\"0\" backwards_edges=\"0\"/>\n";
    xml << "  </volume>\n";
    
    return xml.str();
}

std::string PrusaLib3mfProcessor::generateOutlineVolumeXML(const ModelObjectInfo& objInfo, size_t volumeId) {
    std::ostringstream xml;
    
    xml << "  <volume firstid=\"" << objInfo.start_triangle_index 
        << "\" lastid=\"" << objInfo.end_triangle_index << "\">\n";
    
    xml << "   <metadata type=\"volume\" key=\"name\" value=\"" << objInfo.name << "\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"volume_type\" value=\"ModelPart\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"matrix\" value=\"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_file\" value=\"" << objInfo.name << "\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_object_id\" value=\"0\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_volume_id\" value=\"" << volumeId << "\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_offset_x\" value=\"0\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_offset_y\" value=\"0\"/>\n";
    xml << "   <metadata type=\"volume\" key=\"source_offset_z\" value=\"0\"/>\n";
    xml << "   <mesh edges_fixed=\"0\" degenerate_facets=\"0\" facets_removed=\"0\" facets_reversed=\"0\" backwards_edges=\"0\"/>\n";
    xml << "  </volume>\n";
    
    return xml.str();
}

bool PrusaLib3mfProcessor::generateMetadata(const std::string& extractDir, const ModelConverter& converter, 
                                          const std::vector<MeshInfo>& meshInfos, 
                                          const std::vector<StressDensityMapping>& mappings, 
                                          double maxStress) {
    try {
        // Metadataディレクトリを作成
        std::filesystem::path metadataDir = std::filesystem::path(extractDir) / "Metadata";
        if (!std::filesystem::create_directories(metadataDir)) {
            if (!std::filesystem::exists(metadataDir)) {
                std::cerr << "Failed to create Metadata directory" << std::endl;
                return false;
            }
        }
        
        // XMLファイルのパス
        std::filesystem::path configPath = metadataDir / "Slic3r_PE_model.config";
        
        // XMLコンテンツを生成
        std::ostringstream xmlContent;
        xmlContent << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlContent << "<config>\n";
        xmlContent << " <object id=\"1\" instances_count=\"1\">\n";
        
        const auto& objectInfos = converter.get_object_infos();
        
        // 最初のオブジェクト名をobjectのmetadataとして設定
        if (!objectInfos.empty()) {
            xmlContent << "  <metadata type=\"object\" key=\"name\" value=\"" << objectInfos[0].name << "\"/>\n";
        }
        
        // 各メッシュに対してvolumeタグを生成
        size_t modifierMeshIndex = 0;
        for (size_t i = 0; i < objectInfos.size(); ++i) {
            const auto& objInfo = objectInfos[i];
            
            // modifierMeshかどうかを判定（名前に"modifierMesh"が含まれているかチェック）
            bool isModifierMesh = objInfo.name.find("modifierMesh") != std::string::npos;
            
            if (isModifierMesh) {
                // modifierMeshの場合：fill_density情報を含むParameterModifier
                const MeshInfo* meshInfo = (modifierMeshIndex < meshInfos.size()) ? &meshInfos[modifierMeshIndex] : nullptr;
                xmlContent << generateInfillVolumeXML(objInfo, i, meshInfo, mappings, maxStress);
                modifierMeshIndex++;
            } else {
                // 通常のメッシュの場合：ModelPart
                xmlContent << generateOutlineVolumeXML(objInfo, i);
            }
        }
        
        xmlContent << " </object>\n";
        xmlContent << "</config>";
        
        // ファイルに書き込み
        std::ofstream configFile(configPath);
        if (!configFile.is_open()) {
            std::cerr << "Failed to open config file for writing: " << configPath << std::endl;
            return false;
        }
        
        configFile << xmlContent.str();
        configFile.close();
        
        std::cout << "Generated metadata config file: " << configPath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error generating metadata: " << e.what() << std::endl;
        return false;
    }
}