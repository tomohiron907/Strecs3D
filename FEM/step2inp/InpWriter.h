#ifndef INP_WRITER_H
#define INP_WRITER_H

#include <string>
#include <fstream>

class InpWriter {
public:
    InpWriter();
    ~InpWriter();

    // Initialize INP file (write initial mesh)
    int initializeInpFile(const std::string& step_file, const std::string& inp_file);

    // Open file for appending
    bool openForAppend(const std::string& inp_file);

    // Close file
    void close();

    // Write analysis step configuration
    void writeStep(std::ofstream& f) const;
    void writeOutputs(std::ofstream& f) const;
    void writeEndStep(std::ofstream& f) const;

    // Get base filename without extension
    static std::string getBaseFilename(const std::string& filename);

private:
    std::ofstream file_;
};

#endif // INP_WRITER_H
