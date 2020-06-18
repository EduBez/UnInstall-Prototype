#include "UninstallUnit.h"
#include "FormUnit.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <utility>

using std::copy;
using std::count;
using std::endl;
using std::replace;
using std::runtime_error;
using std::stringstream;
using std::unique;
using std::unique_copy;


//
//  Adapter implementation
//
void Adapter::Adapt(const string& first, const string& second)
{
    // Work with command value in uppercase
    string command = Util::UpperCase(m_value);

    // Look for an executable extension
    string::size_type pos = command.find(".EXE");

    if (pos != string::npos)   // An executable file name was found
    {                          // Extract just the file name
        string file = File::ExtractFileName(command.substr(0, pos + EXT_SIZE));

        if (Compares(first, file)) // Compare engine name with file name
        {
            if (FindGUID(command)) // Special cases
            {       // Microsoft Windows Installer
                if (file == "MSIEXEC.EXE")
                {
                    pos = Util::UpperCase(m_value).find("/I");
                    if (pos != string::npos)
                    {
                        m_value.replace(pos, 2, "/x");// Replace /I with /x
                        m_value += " " + second;      // Insert at end

                        m_isSilent = true;
                        return;
                    }
                }   // InstallShield Professional or InstallShield Developer
                else if (file == "SETUP.EXE" || file == "IDRIVER.EXE")
                {
                    m_value += " " + second;          // Insert at end
                    m_isSilent = true;
                    return;
                }
            }
                                   // Look for parameter in the command string
            string::size_type at = command.find(Util::UpperCase(second));

            if (at == string::npos)// No parameter found
            {                      // Ok to perform adaptation
                m_value.insert(pos + EXT_SIZE, " " + second + " ");
                m_isSilent = true;
            }
        }
    }
}

bool Adapter::Compares(const string& first, const string& second)
{
    string value = Util::UpperCase(first);
    bool flag = false;

    for (unsigned i = 0; i < value.length(); ++i)
    {                    // Ignore wildcard char
        if (value[i] == '?') continue;
        flag = (value[i] == second[i]) ? true : false;
        if (flag == false) break;
    }
    return flag;
}

bool Adapter::FindGUID(const string& command)
{
    bool result = false;
    string::size_type beg, end;

    beg = command.find_first_of("{");
    end = command.find_last_of ("}");

    if (beg != string::npos && end != string::npos)
    {   // Distance must be 37 chars to a valid GUID
        if (end - beg == 37)
        {
            int dash[] = {9, 14, 19, 24};
            for (int i = 0; i < sizeof(dash) / sizeof(int); ++i)
            {
                int at = beg + dash[i];
                result = (command[at] == '-') ? true : false;
                if (!result) break;
            }
        }
    }
    return result;
}


//
//  UninstallLog implementation
//
UninstallLog::UninstallLog()
{
    string path = ParamStr(2).c_str();
    File::ValidatePath(path);
    m_file = path + Util::GetNetBIOSName() + "~UnInstall.Log";
}

void UninstallLog::CacheWrite(const string& section, const string& text)
{
    m_coll.insert(section_value(section, text));
}

void UninstallLog::Flush(const string& section)
{
    vector<char> buff;
    for (section_type::iterator it = m_coll.begin(); it != m_coll.end(); it++)
        if ((*it).first == section)
            if ((*it).second.length())
            {
                for (unsigned i = 0; i < (*it).second.length(); ++i)
                    buff.push_back((*it).second[i]);
                buff.push_back('\n');
            }

    replace(buff.begin(), buff.end(), '\n', '\x0');
    // Must end section data with double null
    buff.push_back('\x0');
    // WIN32 function for writing INI file section
    WritePrivateProfileSection(section.c_str(), &buff[0], m_file.c_str());
    m_coll.erase(section);
}

string UninstallLog::GetSectionName(const string& section)
{
    stringstream stream;
    stream << section << " ~ "
           << (Util::IsPreview() ? "(PREVIEW) [" : "")
           << DateTime::Now();

    return stream.str();
}

void UninstallLog::Write(const string& section,
                         const string& text, bool succeeded)
{
    string dots;
    if (LINE_SIZE > text.length())
        dots = string(LINE_SIZE - text.length(), '.');

    stringstream stream;
    stream << text << " " << dots << " "
           << "["
           << (succeeded ? "OK" : "FAIL")
           << "]";

    CacheWrite(section, stream.str());
}

void UninstallLog::Write(const string& section,
                         const string& text, long count)
{
    string dots;
    if (LINE_SIZE > text.length())
        dots = string(LINE_SIZE - text.length(), '.');

    stringstream stream;
    stream << text << " " << dots << " "
           << "["
           << count
           << "]";

    CacheWrite(section, stream.str());
}


//
//  UninstallList implementation
//
void UninstallList::AdaptString(string& value, bool& isSilent)
{
    Adapter adapter(value);

    for (key_val_type::iterator it = m_engine.begin();
                                it!= m_engine.end(); ++it)
        adapter.Adapt((*it).first, (*it).second);

    value    = adapter.Result();
    isSilent = adapter.IsSilent();
}

void UninstallList::LoadConfig(const string& file)
{
    m_engine.clear();
    string data = File::GetPrivateSection("INSTALLERS", file);

    string::size_type pos;
    while ((pos = data.find("\n")) != string::npos)
    {
        if (pos != string::npos)
        {
            string value = data.substr(0, pos);
            data.erase(0, pos + 1);

            if ((pos = value.find("=")) != string::npos)
                m_engine[value.substr(0, pos)] = value.substr(pos + 1);
        }
    }
}

void UninstallList::LoadDeny(const string& file)
{
    // Load extensions section
    LoadSectionIntoContainer(file, m_denyExt, "EXTENSIONS");

    // Load files section
    LoadSectionIntoContainer(file, m_denyFile, "FILES");

    // Load programs section
    m_deny.clear();
    string data = File::GetPrivateSection("PROGRAMS", file);

    string::size_type pos;
    while ((pos = data.find("\n")) != string::npos)
    {
        if (pos != string::npos)
        {
            string value = data.substr(0, pos);
            data.erase(0, pos + 1);

			if ((pos = value.find("=")) == string::npos)
                m_deny[value] =  string();
            else
                m_deny[value.substr(0, pos)] = value.substr(pos + 1);
        }
    }
}

void UninstallList::LoadGrant(const string& file)
{
    // Using NETBIOS name + section name as section ID
    string section = Util::GetNetBIOSName() + " @ EXTENSIONS";
    // Load extensions section
    LoadSectionIntoContainer(file, m_grantExt, section);

    section = Util::GetNetBIOSName() + " @ FILES";
    // Load files section
    LoadSectionIntoContainer(file, m_grantFile, section);

    section = Util::GetNetBIOSName() + " @ PROGRAMS";
    // Load programs section (no case change)
    LoadSectionIntoContainer(file, m_grant, section, false);
}

void UninstallList::LoadSectionIntoContainer(const string& file,
                                             set<string>& collection,
                                             const string& section,
                                             bool uppercase)
{
    collection.clear();
    string data = File::GetPrivateSection(section, file);

    string::size_type pos;
    while ((pos = data.find("\n")) != string::npos)
    {
        if (pos != string::npos)
        {
            string value = data.substr(0, pos);
            data.erase(0, pos + 1);
            collection.insert(uppercase ? Util::UpperCase(value) : value);
        }
    }
}

void UninstallList::MakeEntry(const string& leaf)
{
    m_registry.Open(HKEY_LOCAL_MACHINE, (HKEY_UNINSTALL + leaf).c_str(), KEY_READ);

    string display;
    // Retrieve <DisplayName> value
    m_registry.ReadString("DisplayName", display);

    // Must have a display name to be valid for uninstallation
    if (display.empty()) return;

    std::set<string>::iterator it;

    it = m_grant.find(display); // Search in the granted collection
    if (it == m_grant.end())    // The item was not found
    {
        key_val_type::const_iterator cit;

        cit = m_deny.find(display); // Search in the denied collection
        if (cit != m_deny.end())    // The item was found
        {
            KeyValue item;

            if (!(*cit).second.empty()) // Look for special case
            {                           // Special case found
                item.IsSilent = true;
                item.QuietUninstallValue = (*cit).second;
            }
            else
            {   // Retrieve <UninstallString> value
                m_registry.ReadString("UninstallString",
                                      item.UninstallValue);

                // Retrieve <QuietUninstallString> value
                m_registry.ReadString("QuietUninstallString",
                                       item.QuietUninstallValue);

                if (!item.UninstallValue.empty() || !item.QuietUninstallValue.empty())
                {
                    // When there's no quiet uninstallation
                    if (item.QuietUninstallValue.empty())
                        // Perform adaptation in the installer's command line
                        AdaptString(item.UninstallValue, item.IsSilent);
                    else
                        item.IsSilent = true;
                }
            }
            // Add to collection
            m_coll[display] = item;
        }
        //MARCEL - Change
        else //Verify files that are missing in both collections-grant and deny
            if(Util::IsDifference()) //If it should be stored.
                m_notBnotW.insert(display); //Stores in the set.
    }
    m_registry.Close();
}

void UninstallList::PopulateList()
{
    m_coll.clear();
    m_name.clear();

    m_registry.Open(HKEY_LOCAL_MACHINE, HKEY_UNINSTALL, KEY_READ);
    m_registry.GetKeyNames(m_name);
    m_registry.Close();

    for (unsigned i = 0; i < m_name.size(); ++i)
    {
        // Try to add an uninstall entry
        MakeEntry(m_name[i]);

        // Update UI
        frmMain->lblProgram->Caption = m_name[i].c_str();
        Application->ProcessMessages();
    }
}


//
//  Uninstall implementation
//

void Uninstall::DisplayScanResults()
{
    bool flag = (m_coll.List().size() || m_file.size() || m_ext.size());
    stringstream stream;

    //// program section ///////////////////////////////////////////////////

    if (m_coll.List().size())
    {
        stream << "[ Programas ]" << endl;

        UninstallList::reg_val_type::const_iterator cit;
        for (cit = m_coll.List().begin(); cit != m_coll.List().end(); cit++)
            stream << (*cit).first << endl;
    }

    //// files section /////////////////////////////////////////////////////

    if (m_file.size())
    {
        stream << endl << "[ Arquivos ]"  << endl;

        vector<string>::iterator it;
        for(it = m_file.begin(); it != m_file.end(); it++)
            stream << File::ExtractFileName(*it) << endl;
    }

    //// extension section /////////////////////////////////////////////////

    if (m_ext.size())
    {
        stream << endl << "[ Extensões ]" << endl;

        vector<string> coll;

        copy(m_ext.begin(), m_ext.end(),
             inserter(coll, coll.begin())); // Make a working copy

        transform(coll.begin(), coll.end(), // Extract the extension
                  coll.begin(),             // from the file name
                  File::ExtractFileExtension);

        sort(coll.begin(), coll.end());     // Sort the collection

        vector<string>::iterator it;
        std::vector<string>::iterator pos;
                                            // Removes consecutive duplicates
        pos = unique(coll.begin(), coll.end());

        for (it = coll.begin(); it != pos; it++)
            stream << *it << endl;
    }

    if (flag) ShowMessage(stream.str().c_str());
}

void Uninstall::ExecCommand(const KeyValue& entry)
{
    if (entry.IsSilent)
    {
        string cmdLine = entry.QuietUninstallValue.empty() ?
            entry.UninstallValue : entry.QuietUninstallValue;

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));

        si.cb          = sizeof(si);
        si.dwFlags     = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        CreateProcess(NULL, const_cast<char*>(cmdLine.c_str()),
                      NULL,
                      NULL,
                      TRUE,
                      0, NULL, NULL, &si, &pi);

        WaitForSingleObject(pi.hProcess, TIMEOUT);
    }
}

void Uninstall::ProcessFiles(const string& dir)
{
    WIN32_FIND_DATA data;  // Structure for file data
    HANDLE handle;         // Handle for searching

    // Search for the files within the dir
    handle = FindFirstFile((dir + "*.*").c_str(), &data);

    if (handle != INVALID_HANDLE_VALUE)
    {
        BOOL result;       // Find next file result
        string name, file, ext;
        std::set<string>::const_iterator it;

        do {
            file = data.cFileName;
            // Uppercase name for searching the collections
            name = Util::UpperCase(file);
            // Search for all the subdirectories within this directory
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                 file != "." && file != "..")

                // Recursive call here
                ProcessFiles(dir + file + "\\");

            else if (file != "." && file != "..")
            {
                // Search in the granted file collection
                it = m_coll.GrantFile().find(name);
                // File not found
                if (it == m_coll.GrantFile().end())
                {
                    ext = File::ExtractFileExtension(name);
                    // Search in the granted extension collection
                    it = m_coll.GrantExt().find(ext);
                    // Extension not found
                    if (it == m_coll.GrantExt().end())
                    {
                        // Search in the denied file collection
                        it = m_coll.DenyFile().find(name);
                        // File not found
                        if (it == m_coll.DenyFile().end())
                        {
                            // Search in the denied extension collection
                            it = m_coll.DenyExt().find(ext);
                            // Extension found
                            if (it != m_coll.DenyExt().end())
                                // Store full path and file name
                                m_ext.push_back(dir + name);
                        }
                        else// File found
                            // Store full path and file name
                            m_file.push_back(dir + name);
                    }
                }
            }
            // Update UI
            frmMain->lblFile->Caption = (dir + file).c_str();
            Application->ProcessMessages();

            result = FindNextFile(handle, &data);
        }
        while (result != 0 && !frmMain->CanClose);
        FindClose(handle);
    }
}

void Uninstall::Remove()
{
    bool erase = !Util::IsPreview();

    m_file.clear();
    m_ext .clear();

    ShowMessage("Iniciando a varredura de Programas e Arquivos");

    //// step 1: scan for programs /////////////////////////////////////////

    m_coll.PopulateList();

    //// step 2: scan for files and extensions /////////////////////////////

    if (m_coll.DenyFile().size() || m_coll.DenyExt().size())
    {
        vector<string> drives = Util::GetFixedDrives();
        std::vector<string>::iterator it = drives.begin();

        for(;it != drives.end(); it++)
            ProcessFiles(*it);
    }

    //MARCEL - Change
    //// Step 2.5: Scan for programs that are missing in both (deny and grant)

    string section = theLog.GetSectionName("PROG_DIFF");
    theLog.Write(section);

    set<string>::const_iterator itD1, itD2;
    itD2 = m_coll.DiffExt().end();

    for(itD1 = m_coll.DiffExt().begin(); itD1 != itD2; ++itD1)
        theLog.Write(section, *itD1, true); //Write program in the log.

    theLog.Flush(section);

    if (!frmMain->CanClose)
    {
        DisplayScanResults();
        if (erase) ShowMessage("Iniciando a remoção de Programas e Arquivos");

    //// step 3: remove programs /////////////////////////////////////////

        section = theLog.GetSectionName("PROGRAMS");
        theLog.Write(section);

        std::set<string> coll;
        UninstallList::reg_val_type::const_iterator cit;

        if (m_coll.List().size())
        {   // Update UI
            frmMain->barProgram->Visible = true;
            frmMain->barProgram->Max = m_coll.List().size();
            frmMain->barProgram->Position = 0;

            Application->ProcessMessages();

            // For all programs in the current collection
            for (cit = m_coll.List().begin();
                 cit!= m_coll.List().end() && !frmMain->CanClose; cit++)
            {
                coll.insert((*cit).first);// Save program name

                if (erase)                // Execute uninstall command
                    ExecCommand((*cit).second);

                // Update UI
                frmMain->barProgram->Position++;
                Application->ProcessMessages();
            }

            m_coll.PopulateList(); // We reload registry info to know what
                                   // programs where uninstalled

            std::set<string>::const_iterator it;
            // For all programs in the previous collection
            for (it = coll.begin(); it != coll.end(); it++)
            {
                string x = (*it);

                cit = m_coll.List().find(*it);  // Verify if program still exists
                if (cit != m_coll.List().end()) // It couldn't be uninstalled
                {
                    if (erase)
                    {   // Log failure
                        theLog.Write(section, *it, false);
                        continue;
                    }
                }
                // Log success
                theLog.Write(section, *it, true);
            }
        }
        theLog.Flush(section);

    //// step 4: remove files ////////////////////////////////////////////

        string file;
        section = theLog.GetSectionName("FILES");

        theLog.Write(section);

        if (m_file.size())
        {   // Update UI
            frmMain->barFile->Visible = true;
            frmMain->barFile->Max = m_file.size();
            frmMain->barFile->Position = 0;

            Application->ProcessMessages();

            vector<string>::iterator it;
            // Process the FILE section
            for(it = m_file.begin();
                it!= m_file.end() && !frmMain->CanClose; it++)
            {
                file = File::ExtractFileName(*it);
                if (erase)
                {
                    if (DeleteFile((*it).c_str()))
                        theLog.Write(section, file, true);
                    else
                        theLog.Write(section, file, false);
                }
                else
                    theLog.Write(section, file, true);

                // Update UI
                frmMain->barFile->Position++;
                Application->ProcessMessages();
            }

        }
        theLog.Flush(section);

    //// step 5: remove extensions ///////////////////////////////////////

        section = theLog.GetSectionName("EXTENSIONS");
        theLog.Write(section);

        if (m_ext.size())
        {
            vector<string>::iterator it;
            // Process the EXTENSION section
            if (erase)
                for (it = m_ext.begin();
                     it!= m_ext.end() && !frmMain->CanClose;)
                {
                    if (!DeleteFile((*it).c_str())) // If can't delete the file
                        it = m_ext.erase(it);       // Remove it from the collection
                }

            vector<string> coll;
            copy(m_ext.begin(), m_ext.end(),
                 inserter(coll, coll.begin()));     // Make a working copy

            transform(coll.begin(), coll.end(),     // Extract the extension
                      coll.begin(),                 // from the file name
                      File::ExtractFileExtension);

            sort(coll.begin(), coll.end());         // Sort the collection

            vector<string> unique;
            unique_copy(coll.begin(), coll.end(),   // Removes consecutive duplicates
                        inserter(unique, unique.begin()));

            // Update UI
            frmMain->barFile->Visible = true;
            frmMain->barFile->Max = unique.size();
            frmMain->barFile->Position = 0;

            Application->ProcessMessages();

            for (it = unique.begin(); it != unique.end(); it++)
            {                                       // Calculate each item count
                long total = count(coll.begin(), coll.end(), *it);
                theLog.Write(section, *it, total);

                // Update UI
                frmMain->barFile->Position++;
                Application->ProcessMessages();
            }
        }
        theLog.Flush(section);
    }
}

