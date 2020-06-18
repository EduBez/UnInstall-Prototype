//
//  Uninstall.cpp
//
//      First  parameter - Engine, white list, black list & file mask paths
//      Second parameter - Log file output path
//      Third  parameter - Enable preview mode [/p]
//      Fourth parameter - Enable block mode   [/b]
//      Fifth  parameter - Log differences     [/d]
//
//      UNINSTALL "C:\Uninstall\Config" "C:\Uninstall\Log" /p /b /d

#include <vcl.h>
#pragma hdrstop

#include "FileUnit.h"
#include "UninstallUnit.h"

USEFORM("FormUnit.cpp", frmMain);


//
//   Main entry point
//
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR argv, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "Automatos (R) Uninstaller";

        if (ParamCount() >= 2)
        {
            Application->CreateForm(__classid(TfrmMain), &frmMain);
            Application->Run();
        }
        else
        {
            AnsiString msg;
            msg = "Automatos (R) Uninstaller \n"
                  "Copyright (C) Automatos Inc 2002. All rights reserved. \n\n"
                  //MARCEL - Change
                  "UNINSTALL <config path> <log path> /p[review mode] /b[lock mode] /d[differences]";

            ShowMessage(msg.c_str());
            return 0;
        }
    }
    catch (Exception &E)
    {
        Application->ShowException(&E);
    }
    catch (...)
    {
        try
        {
            throw Exception("");
        }
        catch (Exception &E)
        {
            Application->ShowException(&E);
        }
    }
    return 0;
}

