#include "InpWriter.h"
#include <gmsh.h>
#include <iostream>
#include <filesystem>

InpWriter::InpWriter() {
}

InpWriter::~InpWriter() {
    if (file_.is_open()) {
        file_.close();
    }
}

std::string InpWriter::getBaseFilename(const std::string& filename) {
    std::filesystem::path path(filename);
    return path.stem().string();
}

int InpWriter::initializeInpFile(const std::string& step_file, const std::string& inp_file) {
    try {
        std::cout << "INPファイルを出力中: " << inp_file << std::endl;
        gmsh::write(inp_file);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "INPファイル出力エラー: " << e.what() << std::endl;
        return 1;
    }
}

bool InpWriter::openForAppend(const std::string& inp_file) {
    file_.open(inp_file, std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "エラー: ファイルを開けませんでした: " << inp_file << std::endl;
        return false;
    }
    return true;
}

void InpWriter::close() {
    if (file_.is_open()) {
        file_.close();
    }
}

void InpWriter::writeStep(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** At least one step is needed to run an CalculiX analysis of FreeCAD\n";
    f << "*STEP, INC=2000\n";
    f << "*STATIC\n";
}

void InpWriter::writeOutputs(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Outputs --> frd file\n";
    f << "*NODE FILE\n";
    f << "U\n";
    f << "*EL FILE\n";
    f << "S, E\n";
    f << "** outputs --> dat file\n";
    f << "** reaction forces for Constraint fixed\n";
    f << "*NODE PRINT, NSET=ConstraintFixed, TOTALS=ONLY\n";
    f << "RF\n";
}

void InpWriter::writeEndStep(std::ofstream& f) const {
    f << "*OUTPUT, FREQUENCY=1\n";
    f << "***********************************************************\n";
    f << "*END STEP\n";
}
