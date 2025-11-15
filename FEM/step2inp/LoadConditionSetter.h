#ifndef LOAD_CONDITION_SETTER_H
#define LOAD_CONDITION_SETTER_H

#include <vector>
#include <fstream>

struct LoadProperties {
    int surface_number;
    double magnitude;
    std::vector<double> direction;
};

class LoadConditionSetter {
public:
    LoadConditionSetter();
    ~LoadConditionSetter();

    // Add load condition
    void addLoad(const LoadProperties& load);
    void addLoad(int surface_number, double magnitude, const std::vector<double>& direction);

    // Calculate element area (geometry utility)
    static double calculateElementArea(const std::vector<std::vector<double>>& coords);

    // Write load boundary conditions
    void writeForceBoundaryCondition(std::ofstream& f, int surface_number,
                                     double total_force,
                                     const std::vector<double>& force_direction) const;

    // Get all load conditions
    const std::vector<LoadProperties>& getLoads() const;

private:
    std::vector<LoadProperties> loads_;
};

// Utility function
LoadProperties createLoadCondition(int surface_number, double magnitude,
                                   const std::vector<double>& direction);

#endif // LOAD_CONDITION_SETTER_H
