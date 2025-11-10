#ifndef FRD2VTU_H
#define FRD2VTU_H

#include <string>

/**
 * Convert FRD file to VTU format
 * @param frd_filename Input FRD file path
 * @param vtu_filename Output VTU file path
 * @return 0 on success, non-zero on error
 */
int convertFrdToVtu(const std::string& frd_filename, const std::string& vtu_filename);

#endif // FRD2VTU_H