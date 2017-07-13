#ifndef __CSV_PARSER_H__
#define __CSV_PARSER_H__

#include "../defines.h"
#include <fstream>
#include <cstdlib>

class csvParser
{
public:
    csvParser();
    ~csvParser();
    const bool openFile( const SString& filename );
    const SString nextLine();
    const SVector< SString > split( const SString& str, char delimiter );
private:
    std::ifstream csv_file;
};

#endif