#define WIN32_LEAN_AND_MEAN
#include <vcl.h>
#pragma hdrstop

#include "FileUnit.h"
#include "SystemUnit.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

using std::ifstream;
using std::runtime_error;
using std::stringstream;


//
//  File implementation
//
string File::ExtractFileExtension(const string& file)
{
    string ext;
    string::size_type pos = file.find_last_of('.');
    if (pos != string::npos)
        ext = file.substr(pos + 1);
    return ext;
}

string File::ExtractFileName(const string& file)
{
    string name;
    string::size_type at1, at2, size;

    at1 = file.find('\\');
    at2 = file.find('.');

    if (at1 != string::npos && at2 != string::npos)
    {
        at1 = file.find_last_of('\\');
        at2 = file.find_last_of('.');

        if (at2 > at1)
        {
            size = at2 - at1;
            name = file.substr(at1 + 1, size) + file.substr(at2 + 1);
        }
        else
            name = file.substr(at1 + 1);
    }
    else if (at1 != string::npos && at2 == string::npos)
    {
        at1 = file.find_last_of('\\');
        name = file.substr(at1 + 1);
    }
    else
        name = file;
    return name;
}

string File::ExtractFilePath(const string& file)
{
    string path;
    string::size_type pos = file.find_last_of('\\');
    if (pos != string::npos)
        path = file.substr(0, pos + 1);
    return path;
}

bool File::FileExists(const string& file)
{
    bool flag = false;

    WIN32_FIND_DATA data;
	HANDLE handle = FindFirstFile(file.c_str(), &data);

    if (handle != INVALID_HANDLE_VALUE)
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            flag = true;

    FindClose(handle);
	return flag;
}

string File::GetPrivateSection(const string& section, const string& file)
{
    char buff[SECTION_SIZE] = "";
    // WIN32 function for reading INI file section
    DWORD size = GetPrivateProfileSection(section.c_str(),
                                         buff,
                                         sizeof(buff),
                                         file.c_str());
    // Section size can't exceed 32 Kb
    if (size == SECTION_SIZE - 2)
    {
        stringstream stream;
        stream << "A seção \""
               << section
               << "\" no arquivo \""
               << file
               << "\" é maior do que 32Kb";

        throw runtime_error(stream.str());
    }

    for (unsigned i=0; i < size; ++i) {
        if (buff[i] == '\x0') buff[i] = '\n';
	}
    return string(buff);
}

string File::LoadFile(const string& file)
{
    string content;             // String to hold stream contents
    ifstream in(file.c_str());  // Open file

    if (in.is_open())
    {
        stringstream stream;    // Stream for reading from file
        stream << in.rdbuf();   // Get file data into the stream
        content = stream.str(); // Convert it to a string
    }
    return content;
}

void File::ValidatePath(string& path)
{
    if (path.empty()) return;
    if (path.at(path.length()-1) != '\\') path.insert(path.length(), "\\");
}

