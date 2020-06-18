#ifndef FILE_UNIT_H
    #define FILE_UNIT_H

#include <string>
using std::string;


//
//  File
//
struct File
{
    static string ExtractFileExtension(const string& file);
    static string ExtractFileName(const string& file);
    static string ExtractFilePath(const string& file);

    static bool FileExists(const string& file);

    static string GetPrivateSection(const string& section, const string& file);
    static string LoadFile(const string& file);

    static void ValidatePath(string& path);

private:
    enum {SECTION_SIZE=32767};// 32Kb
};

#endif

