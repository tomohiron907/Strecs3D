#include "StepTransformer.h"
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS_Shape.hxx>
#include <iostream>

bool StepTransformer::transformAndSave(const std::string& inputPath, 
                                       const std::string& outputPath, 
                                       const gp_Trsf& transform) {
    // 1. Read the STEP file
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(inputPath.c_str());
    if (status != IFSelect_RetDone) {
        std::cerr << "Error: Failed to read STEP file: " << inputPath << std::endl;
        return false;
    }

    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        std::cerr << "Error: STEP file contains no shape: " << inputPath << std::endl;
        return false;
    }

    // 2. Apply transformation (Rotation / Translation)
    // Copy=True ensures the original shape is not modified in place (though not strictly necessary here since we just read it)
    BRepBuilderAPI_Transform transformer(shape, transform, Standard_True);
    TopoDS_Shape newShape = transformer.Shape();

    // 3. Write to a new STEP file
    STEPControl_Writer writer;
    IFSelect_ReturnStatus transferStatus = writer.Transfer(newShape, STEPControl_AsIs);
    
    if (transferStatus != IFSelect_RetDone) {
        std::cerr << "Error: Failed to transfer shape to STEP writer." << std::endl;
        return false;
    }

    status = writer.Write(outputPath.c_str());
    if (status != IFSelect_RetDone) {
        std::cerr << "Error: Failed to write STEP file: " << outputPath << std::endl;
        return false;
    }

    std::cout << "Successfully transformed STEP file: " << inputPath 
              << " -> " << outputPath << std::endl;
    return true;
}
