#include "csvParser.h"
#include <sstream>

csvParser::csvParser()
{}

csvParser::~csvParser()
{
    if( csv_file.is_open() )
    {
        csv_file.close();
    }
}

const bool csvParser::openFile( const SString& filename )
{
    csv_file.open( filename.c_str() );
    return csv_file.is_open();
}

const SString csvParser::nextLine()
{
    SString line;
    if( !csv_file.eof() )
    {
        getline( csv_file , line );
    }
    return line;
}

const SVector< SString > csvParser::split( const SString& str, char delimiter) 
{
    SVector< SString > internal;
    stringstream ss( str );
    SString tok;
    while( getline( ss , tok , delimiter ) ) 
    {
        internal.push_back( tok );
    }
    return internal;
}