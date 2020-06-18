#define WIN32_LEAN_AND_MEAN
#include <vcl.h>
#pragma hdrstop

#include "SystemUnit.h"

#include <algorithm>
#include <memory>

using std::advance;
using std::auto_ptr;
using std::transform;


//
//  Util implementation
//
bool Util::IsBlockMode()
{
    return (LowerCase(ParamStr(3).c_str()) == "/b" ||
            //MARCEL - Change
            LowerCase(ParamStr(4).c_str()) == "/b" ||
            LowerCase(ParamStr(5).c_str()) == "/b");
}

bool Util::IsPreview()
{
    return (LowerCase(ParamStr(3).c_str()) == "/p" ||
            //MARCEL - Change
            LowerCase(ParamStr(4).c_str()) == "/p" ||
            LowerCase(ParamStr(5).c_str()) == "/p");
}

//MARCEL - Change
bool Util::IsDifference()
{
    return (LowerCase(ParamStr(3).c_str()) == "/d" ||
            LowerCase(ParamStr(4).c_str()) == "/d" ||
            LowerCase(ParamStr(5).c_str()) == "/d");
}

vector<string> Util::GetFixedDrives()
{
    char buff[MAX_PATH] = "";
    DWORD size = GetLogicalDriveStrings(sizeof(buff), buff);

    for (unsigned i=0; i < size; ++i) {
        if (buff[i] == '\x0') buff[i] = '\n';
	}
	string data(buff);

	vector<string> cont;
    string::size_type pos;

    while ((pos = data.find("\n")) != string::npos)
        if (pos != string::npos)
        {
            string value = data.substr(0, pos);
            data.erase(0, pos + 1);

			UINT type = GetDriveType(value.c_str());

			if (type == DRIVE_FIXED)
				cont.push_back(value);
        }
    return cont;
}

string Util::GetNetBIOSName()
{
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    char buff[MAX_COMPUTERNAME_LENGTH + 1] = "";
    GetComputerName(buff, &size);
    return string(buff);
}

string Util::LowerCase(const string& value)
{
    string text = value;
    transform(text.begin(), text.end(), text.begin(), tolower);
    return text;
}

string Util::UpperCase(const string& value)
{
    string text = value;
    transform(text.begin(), text.end(), text.begin(), toupper);
    return text;
}

string Util::ProperCase(const string& value)
{
    string text = value;
    string::size_type at, pos;

    at = 0;
    // Search for first word
    pos = text.find(' ');

    // For each word found
    while (pos != string::npos)
    {
        // Upper case first char
        text[at] = toupper(text[at]);

        // Advance to next word
        at  = pos + 1;
        pos = text.find(' ', at);
    }
    // Handle last word
    text[at] = toupper(text[at]);
    return text;
}

string Util::Trim(const string& value)
{
    string text = value;
    string::size_type pos;
    string::iterator it = text.begin();

    if (text[0] == ' ')               // First char is white space
    {                                 // Find first char not equal to white space
        pos = text.find_first_not_of(' ');
        advance(it, pos);             // Adjust iterator to point to this position
        text.erase(text.begin(), it); // Erase this interval
    }

    it  = text.begin();               // Rewind iterator
    pos = text.find_last_not_of(' '); // Find last char not equal to white space

    advance(it, pos + 1);             // Adjust iterator to point to this position
    text.erase(it, text.end());       // Erase this interval
    return text;
}

