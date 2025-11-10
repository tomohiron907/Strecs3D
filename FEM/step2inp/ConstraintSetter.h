#ifndef CONSTRAINT_SETTER_H
#define CONSTRAINT_SETTER_H

#include <vector>
#include <fstream>

struct ConstraintCondition {
    int surface_number;
};

class ConstraintSetter {
public:
    ConstraintSetter();
    ~ConstraintSetter();

    // Add constraint condition
    void addConstraint(const ConstraintCondition& constraint);
    void addConstraint(int surface_number);

    // Get node tags for a surface
    std::vector<int> getConstraintNodeTags(int surface_number) const;

    // Write constraint node sets to file
    void writeConstraintNodeSet(std::ofstream& f, int surface_number) const;
    void writeFixedConstraints(std::ofstream& f) const;

    // Get all constraints
    const std::vector<ConstraintCondition>& getConstraints() const;

private:
    std::vector<ConstraintCondition> constraints_;
};

// Utility function
ConstraintCondition createConstraintCondition(int surface_number);

#endif // CONSTRAINT_SETTER_H
