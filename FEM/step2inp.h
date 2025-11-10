#ifndef STEP2INP_H
#define STEP2INP_H

#include <string>
#include <vector>
#include <fstream>
#include "step2inp/MeshGenerator.h"
#include "step2inp/ConstraintSetter.h"
#include "step2inp/MaterialSetter.h"
#include "step2inp/LoadConditionSetter.h"
#include "step2inp/InpWriter.h"

class Step2Inp {
public:
    Step2Inp();
    ~Step2Inp();

    // Main conversion method
    int convert(const std::string& step_file,
                const std::vector<ConstraintCondition>& constraints,
                const std::vector<LoadCondition>& loads);

    // Access to components for advanced usage
    MeshGenerator& getMeshGenerator() { return mesh_generator_; }
    ConstraintSetter& getConstraintSetter() { return constraint_setter_; }
    MaterialSetter& getMaterialSetter() { return material_setter_; }
    LoadConditionSetter& getLoadConditionSetter() { return load_setter_; }
    InpWriter& getInpWriter() { return inp_writer_; }

private:
    // Component objects
    MeshGenerator mesh_generator_;
    ConstraintSetter constraint_setter_;
    MaterialSetter material_setter_;
    LoadConditionSetter load_setter_;
    InpWriter inp_writer_;
};

// Utility function
int convertStepToInp(const std::string& step_file,
                     const std::vector<ConstraintCondition>& constraints,
                     const std::vector<LoadCondition>& loads);

#endif // STEP2INP_H
