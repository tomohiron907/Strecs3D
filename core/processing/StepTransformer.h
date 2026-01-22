#pragma once

#include <string>
#include <gp_Trsf.hxx>

class StepTransformer {
public:
    /**
     * @brief Reads a STEP file, applies a transformation, and saves it to a new file.
     * 
     * @param inputPath Path to the input STEP file.
     * @param outputPath Path to save the transformed STEP file.
     * @param transform The transformation (matrix) to apply.
     * @return true if successful, false otherwise.
     */
    static bool transformAndSave(const std::string& inputPath, 
                                 const std::string& outputPath, 
                                 const gp_Trsf& transform);
};
