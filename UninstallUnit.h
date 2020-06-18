#ifndef UNINSTALL_UNIT_H
    #define UNINSTALL_UNIT_H

#include "FileUnit.h"
#include "RegistryUnit.h"
#include "SystemUnit.h"

#include <map>
#include <set>
#include <vector>

using std::map;
using std::multimap;
using std::string;
using std::set;
using std::vector;

// Global object initialized only when used
#define theLog UninstallLog::GetObject()


//
//  Adapter
//
class Adapter
{
    enum {EXT_SIZE=4};
public:
    // Overloaded ctor
    Adapter(const string value) : m_value(value), m_isSilent(false) { };

    void Adapt(const string& first, const string& second);

    const string Result() const {return m_value;}
    bool IsSilent() const {return m_isSilent;}

private:
    bool Compares(const string& first, const string& second);
    bool FindGUID(const string& command);

    string m_value;
    bool m_isSilent;
};


//
//  KeyValue
//
struct KeyValue
{
    // Default ctor
    KeyValue() : IsSilent(false) { };

    bool IsSilent;
    string UninstallValue;
    string QuietUninstallValue;
};


//
//  UninstallLog
//
class UninstallLog
{
    enum {LINE_SIZE=110};

    typedef multimap<string, string> section_type;
    typedef section_type::value_type section_value;

public:
    // Default ctor
    UninstallLog();

    void CacheWrite(const string& section, const string& text);
    void Flush(const string& section);

    inline const string GetLogFileName() const
        {return m_file;}

    inline static UninstallLog& GetObject()
        {static UninstallLog theLog_; return theLog_;}

    string GetSectionName(const string& section);

	inline void Write(const string& section)
        {CacheWrite(section, string());}

	inline void Write(const string& section, const string& text)
        {CacheWrite(section, text);}

    void Write(const string& section, const string& text, bool succeeded);
    void Write(const string& section, const string& text, long count);

private:
    string m_file;
    section_type m_coll;
};


//
//  UninstallList
//
class UninstallList
{
public:
    typedef map<string, string>   key_val_type;
    typedef map<string, KeyValue> reg_val_type;

    // Default ctor
    UninstallList() :
        HKEY_UNINSTALL("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\")
    { };

    void LoadConfig(const string& file);
    void LoadDeny  (const string& file);
    void LoadGrant (const string& file);

    void LoadSectionIntoContainer(const string& file,
                                  set<string>& collection,
                                  const string& section,
                                  bool uppercase=true);

    inline const reg_val_type& List()     const
        {return m_coll;}

    inline const set<string>& DenyExt()   const
        {return m_denyExt;}

    inline const set<string>& DenyFile()  const
        {return m_denyFile;}

    inline const set<string>& GrantExt()  const
        {return m_grantExt;}

    inline const set<string>& GrantFile() const
        {return m_grantFile;}

    //MARCEL - Change
    inline const set<string>& DiffExt()   const
        {return m_notBnotW;}

    void PopulateList();

private:
    void AdaptString(string& value, bool& isSilent);
    void MakeEntry(const string& leaf);

    const char* HKEY_UNINSTALL;

    Registry m_registry;
    vector<string> m_name;

    reg_val_type m_coll;

    key_val_type m_engine;
    key_val_type m_deny;

    set<string> m_denyExt;
    set<string> m_denyFile;
    set<string> m_grant;
    set<string> m_grantExt;
    set<string> m_grantFile;
    //MARCEL - Change
    set<string> m_notBnotW;

};


//
//  Uninstall
//
class Uninstall
{              // 1min
    enum {TIMEOUT=60000};
public:
    // Default ctor
    Uninstall(UninstallList& list) : m_coll(list) { };
    void Remove();

private:
    void DisplayScanResults();
    void ExecCommand(const KeyValue& entry);
    void ProcessFiles(const string& directory);

    UninstallList& m_coll;

    vector<string> m_file;
    vector<string> m_ext;
};

#endif

