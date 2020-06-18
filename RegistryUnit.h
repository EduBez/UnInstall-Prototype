#ifndef REGISTRY_UNIT_H
    #define REGISTRY_UNIT_H

#include <atlbase.h>
#include <string>
#include <vector>

using std::string;
using std::vector;


//
//  Registry
//
class Registry : public CRegKey
{
    enum {BUFF_SIZE=512};
public:
    struct TRegKeyInfo
    {
        DWORD NumSubKeys;
        DWORD MaxSubKeyLen;
        DWORD NumValues;
        DWORD MaxValueLen;
        DWORD MaxDataLen;
        FILETIME FileTime;
    };
    // Default ctor
    Registry() : CRegKey() { };

    bool GetKeyInfo(TRegKeyInfo& value);
    void GetKeyNames(vector<string>& keys);
    void ReadString(const char* key, string& value);
};

#endif
