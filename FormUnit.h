#ifndef FormUnitH
#define FormUnitH

#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <Forms.hpp>
#include <StdCtrls.hpp>

// User defined message
#define WM_START_UNINSTALL   WM_USER + 1


//
//  TfrmMain declaration
//
class TfrmMain : public TForm
{
__published:	// IDE-managed Components
    TButton *btnCancel;

    TGroupBox *grpProgram;
    TGroupBox *grpFile;

    TLabel *lblProgram;
    TLabel *lblFile;

    TProgressBar *barProgram;
    TProgressBar *barFile;

    void __fastcall OnCancel_Click(TObject *Sender);
    void __fastcall OnForm_CloseQuery(TObject *Sender, bool &CanClose);

private:    	// User declarations

protected:
    void virtual __fastcall StartUninstall(TMessage& Msg);

    BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(WM_START_UNINSTALL, TMessage, StartUninstall)
    END_MESSAGE_MAP(TForm)

public:		    // User declarations
    __fastcall TfrmMain(TComponent* Owner);
    bool CanClose;
};

extern PACKAGE TfrmMain *frmMain;
#endif

