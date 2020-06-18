#include "FormUnit.h"
#include "FileUnit.h"
#include "UninstallUnit.h"

#include <stdexcept>
#include <sstream>

using std::runtime_error;
using std::stringstream;

#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;


//
//  TfrmMain implementation
//
__fastcall TfrmMain::TfrmMain(TComponent* Owner) : TForm(Owner)
{
    PostMessage(Handle, WM_START_UNINSTALL, 0, 0L);
}

void __fastcall TfrmMain::OnCancel_Click(TObject *Sender)
{
    this->CanClose = true;
}

void __fastcall TfrmMain::OnForm_CloseQuery(TObject *Sender, bool &CanClose)
{
    CanClose = this->CanClose;
}

void __fastcall TfrmMain::StartUninstall(TMessage& Msg)
{
    try
    {
        try
        {
            string path = ParamStr(1).c_str();
            File::ValidatePath(path);

            string engine = path + "ENGINE.INI"  ;
            string deny   = path + "BLACKLST.INI";
            string grant  = path + "WHITELST.INI";

            UninstallList theList;

            if (File::FileExists(engine))   // These are the mandatory files
            {                               // They both must exist
                if (File::FileExists(deny))
                {
                    theList.LoadConfig(engine); // Load engine data
                    theList.LoadDeny(deny);     // Load deny list data
                }
                else
                {
                    stringstream stream;
                    stream << "Arquivo de configuração \""
                           << deny
                           << "\" não encontrado";

                    throw runtime_error(stream.str());
                }
            }
            else
            {
                stringstream stream;
                stream << "Arquivo de configuração \""
                       << engine
                       << "\" não encontrado";

                throw runtime_error(stream.str());
            }
                                         // This file is optional, but
            if (File::FileExists(grant)) // it requires the others
                theList.LoadGrant(grant);// Load grant list data

            this->CanClose = false;
            if (Util::IsBlockMode()) btnCancel->Visible = true;

            Uninstall theUninstall(theList);
            theUninstall.Remove();
        }
        catch (const std::exception& E)
        {
            theLog.Write(theLog.GetSectionName("Runtime Exception"),
                         E.what(), false);
        }
        catch (Exception &E)
        {
            Application->ShowException(&E);
        }
    }
    __finally
    {
        try
        {
            stringstream stream;
            stream << "Log gerado em \""
                   << theLog.GetLogFileName()
                   << "\"\n\nEncerrando a aplicação";

            ShowMessage(stream.str().c_str());
        }
        catch (...)
        {
        }

        this->CanClose = true;
        Close();
    }
}

