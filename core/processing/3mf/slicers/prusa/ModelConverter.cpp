#include "ModelConverter.h"
#include "pugixml.hpp"
#include <iostream>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <cstdint>

bool ModelConverter::process(const std::string& input_path, const std::string& output_path) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(input_path.c_str());

    if (!result) {
        std::cerr << "Error: Failed to parse XML file '" << input_path << "'.\n";
        std::cerr << "Description: " << result.description() << std::endl;
        return false;
    }

    // Get the first <object> node to merge into.
    pugi::xml_node first_object = doc.select_node("/model/resources/object").node();
    if (!first_object) {
        std::cerr << "Error: No <object> tag found in the model." << std::endl;
        return false;
    }

    pugi::xml_node first_vertices = first_object.child("mesh").child("vertices");
    pugi::xml_node first_triangles = first_object.child("mesh").child("triangles");
    if (!first_vertices || !first_triangles) {
        std::cerr << "Error: Mesh data (<vertices> or <triangles>) not found in the first object." << std::endl;
        return false;
    }

    // Initialize vertex and triangle counts from the first object.
    size_t vertex_offset = std::distance(first_vertices.children("vertex").begin(), first_vertices.children("vertex").end());
    size_t total_triangle_count = std::distance(first_triangles.children("triangle").begin(), first_triangles.children("triangle").end());

    // Record info for the first object.
    object_infos_.clear();
    ModelObjectInfo first_info;
    first_info.name = first_object.attribute("name").as_string();
    first_info.start_triangle_index = 0;
    first_info.end_triangle_index = (total_triangle_count > 0) ? total_triangle_count - 1 : 0;
    object_infos_.push_back(first_info);

    // Get all <object> elements AFTER the first one and store them in a vector
    // to avoid iterator invalidation during node removal
    pugi::xpath_node_set objects_xpath = doc.select_nodes("/model/resources/object[position()>1]");
    std::vector<pugi::xml_node> objects_to_merge;
    objects_to_merge.reserve(objects_xpath.size());
    for (const auto& xpath_node : objects_xpath) {
        objects_to_merge.push_back(xpath_node.node());
    }

    pugi::xml_node build_node = doc.child("model").child("build");

    // Loop through the remaining objects and merge them.
    for (pugi::xml_node current_object : objects_to_merge) {
        pugi::xml_node current_vertices = current_object.child("mesh").child("vertices");
        pugi::xml_node current_triangles = current_object.child("mesh").child("triangles");

        if (!current_vertices || !current_triangles) continue;

        // Append vertices to the first object's vertex list.
        for (pugi::xml_node vertex : current_vertices.children("vertex")) {
            first_vertices.append_copy(vertex);
        }

        // Append triangles, adjusting their indices as we go.
        size_t triangles_in_current_object = 0;
        for (pugi::xml_node triangle : current_triangles.children("triangle")) {
            for (pugi::xml_attribute attr : triangle.attributes()) {
                std::uint64_t original_index = attr.as_ullong();
                // Check for potential overflow
                if (original_index > SIZE_MAX - vertex_offset) {
                    std::cerr << "Error: Index overflow detected during triangle merging." << std::endl;
                    return false;
                }
                attr.set_value(original_index + vertex_offset);
            }
            first_triangles.append_copy(triangle);
            triangles_in_current_object++;
        }

        // Record info for the merged object.
        ModelObjectInfo info;
        info.name = current_object.attribute("name").as_string();
        info.start_triangle_index = total_triangle_count;
        info.end_triangle_index = total_triangle_count + triangles_in_current_object - 1;
        object_infos_.push_back(info);

        // Update the running counts.
        vertex_offset += std::distance(current_vertices.children("vertex").begin(), current_vertices.children("vertex").end());
        total_triangle_count += triangles_in_current_object;

        // Remove the corresponding <item> from the <build> section.
        if (build_node) {
             std::string object_id_to_remove = current_object.attribute("id").as_string();
             pugi::xml_node item_to_remove = build_node.find_child_by_attribute("item", "objectid", object_id_to_remove.c_str());
             if (item_to_remove) {
                 build_node.remove_child(item_to_remove);
             }
        }

        // Remove the now-merged <object> node.
        first_object.parent().remove_child(current_object);
    }

    // <build>セクションに残った最初の<item>のtransform属性を修正します。
    if (build_node) {
        pugi::xml_node final_item = build_node.child("item");
        if (final_item) {
            // transform属性を指定された値に設定
            final_item.attribute("transform").set_value("1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
        }
    }
    
    // Save the final, merged model to the output file.
    if (!doc.save_file(output_path.c_str(), "	", pugi::format_default, pugi::encoding_utf8)) {
        std::cerr << "Error: Failed to save the output file '" << output_path << "'." << std::endl;
        return false;
    }

    return true;
}

const std::vector<ModelObjectInfo>& ModelConverter::get_object_infos() const {
    return object_infos_;
}