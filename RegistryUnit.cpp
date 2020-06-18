#include "RegistryUnit.h"

#include <algorithm>
#include <functional>

using std::greater;
using std::sort;


//
//	Registry implementation
//
bool Registry::GetKeyInfo(TRegKeyInfo& value)
{
    ZeroMemory(&value, sizeof(value));
    return RegQueryInfoKey(m_hKey              ,
                           NULL                ,
                           NULL                ,
                           NULL                ,
                           &value.NumSubKeys   ,
                           &value.MaxSubKeyLen ,
                           NULL                ,
                           &value.NumValues    ,
                           &value.MaxValueLen  ,
                           &value.MaxDataLen   ,
                           NULL                ,
                           &value.FileTime) == ERROR_SUCCESS;
}

void Registry::GetKeyNames(vector<string>& keys)
{
    TRegKeyInfo info;
    if (GetKeyInfo(info))
    {
        for (unsigned i=0; i < info.NumSubKeys; ++i)
        {
            char buff[BUFF_SIZE] = "";
            DWORD size = BUFF_SIZE;
            RegEnumKeyEx(m_hKey, i, buff, &size, NULL, NULL, NULL, NULL);
            if (strlen(buff)) keys.push_back(buff);
        }
    }
    sort(keys.begin(), keys.end(), greater<string>());
}

void Registry::ReadString(const char* key, string& value)
{
    char buff[BUFF_SIZE] = "";
    DWORD size = BUFF_SIZE;
    QueryValue(buff, key, &size);
    if (strlen(buff)) value = buff;
}
