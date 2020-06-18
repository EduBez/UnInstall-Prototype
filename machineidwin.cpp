// IPHLPAPI
//cl netstatx.cpp getopt.obj advapi32.lib Ws2_32.lib

#include <windows.h>
#include <winbase.h>
#include <tchar.h> // for _tprintf()
#include <stdio.h> // for _tprintf()

#include "stdafx.h"
#include "machineid.h"

#include <lm.h>

int  bDEBUG = 0;

DWORD g_dwSleep   = 0;
BOOL  g_bDELTA    = FALSE;
BOOL  g_bCOMPACT  = FALSE;
BOOL  g_bNOLOOPBACK  = FALSE;

#define ANY_SIZE 1
#define MAX_INTERFACE_NAME_LEN 256
#define MAXLEN_IFDESCR 256
#define MAXLEN_PHYSADDR 8


#define IF_OPER_STATUS_NON_OPERATIONAL  0
#define IF_OPER_STATUS_UNREACHABLE      1
#define IF_OPER_STATUS_DISCONNECTED     2
#define IF_OPER_STATUS_CONNECTING       3
#define IF_OPER_STATUS_CONNECTED        4
#define IF_OPER_STATUS_OPERATIONAL      5

#define MIB_IF_TYPE_OTHER               1
#define MIB_IF_TYPE_ETHERNET            6
#define MIB_IF_TYPE_TOKENRING           9
#define MIB_IF_TYPE_FDDI                15
#define MIB_IF_TYPE_PPP                 23
#define MIB_IF_TYPE_LOOPBACK            24
#define MIB_IF_TYPE_SLIP                28

#define MIB_IF_ADMIN_STATUS_UP          1
#define MIB_IF_ADMIN_STATUS_DOWN        2
#define MIB_IF_ADMIN_STATUS_TESTING     3

#define MIB_IF_OPER_STATUS_NON_OPERATIONAL      0
#define MIB_IF_OPER_STATUS_UNREACHABLE          1
#define MIB_IF_OPER_STATUS_DISCONNECTED         2
#define MIB_IF_OPER_STATUS_CONNECTING           3
#define MIB_IF_OPER_STATUS_CONNECTED            4
#define MIB_IF_OPER_STATUS_OPERATIONAL          5




//===================================================================================
typedef struct _MIB_IPADDRROW {
    DWORD   dwAddr;              // IP address
    DWORD   dwIndex;             // interface index
    DWORD   dwMask;              // subnet mask
    DWORD   dwBCastAddr;         // broadcast address 
    DWORD   dwReasmSize;         // rassembly size 
    unsigned short   unused1;    // not currently used 
    unsigned short   unused2;    // not currently used 
} MIB_IPADDRROW, *PMIB_IPADDRROW;

//===================================================================================
typedef struct _MIB_IPADDRTABLE {
    DWORD         dwNumEntries;    // number of entries in the table
    MIB_IPADDRROW table[ANY_SIZE]; // array of IP address entries
} MIB_IPADDRTABLE, *PMIB_IPADDRTABLE;

//===================================================================================
typedef struct _MIB_IFROW {
    WCHAR   wszName[MAX_INTERFACE_NAME_LEN];
    DWORD   dwIndex;    // index of the interface
    DWORD   dwType;     // type of interface
    DWORD   dwMtu;      // max transmission unit 
    DWORD   dwSpeed;    // speed of the interface 
    DWORD   dwPhysAddrLen;    // length of physical address
    BYTE    bPhysAddr[MAXLEN_PHYSADDR]; // physical address of adapter
    DWORD   dwAdminStatus;    // administrative status
    DWORD   dwOperStatus;     // operational status
    DWORD   dwLastChange;     // last time operational status changed 
    DWORD   dwInOctets;       // octets received
    DWORD   dwInUcastPkts;    // unicast packets received 
    DWORD   dwInNUcastPkts;   // non-unicast packets received 
    DWORD   dwInDiscards;     // received packets discarded 
    DWORD   dwInErrors;       // erroneous packets received 
    DWORD   dwInUnknownProtos;  // unknown protocol packets received 
    DWORD   dwOutOctets;      // octets sent 
    DWORD   dwOutUcastPkts;   // unicast packets sent 
    DWORD   dwOutNUcastPkts;  // non-unicast packets sent 
    DWORD   dwOutDiscards;    // outgoing packets discarded 
    DWORD   dwOutErrors;      // erroneous packets sent 
    DWORD   dwOutQLen;        // output queue length 
    DWORD   dwDescrLen;       // length of bDescr member 
    BYTE    bDescr[MAXLEN_IFDESCR];  // interface description 
} MIB_IFROW,*PMIB_IFROW;

typedef struct _MIB_IFTABLE {
    DWORD     dwNumEntries;       // number of entries in table
    MIB_IFROW table[ANY_SIZE];    // array of interface entries
} MIB_IFTABLE, *PMIB_IFTABLE;


typedef DWORD (WINAPI *PFnGetIpAddrTable ) ( PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder);
typedef DWORD (WINAPI *PFnGetNumberOfInterfaces) (PDWORD pdwNumIf);
typedef DWORD (WINAPI *PFnGetFriendlyIfIndex) (DWORD IfIndex);
typedef DWORD (WINAPI *PFnGetIfEntry) (PMIB_IFROW pIfRow);
typedef DWORD (WINAPI *PFnGetIfTable) (PMIB_IFTABLE pIfTable,PULONG pdwSize, BOOL bOrder);

PFnGetNumberOfInterfaces GetNumberOfInterfaces = NULL; 
PFnGetFriendlyIfIndex    GetFriendlyIfIndex    = NULL; 
PFnGetIfEntry            GetIfEntry            = NULL; 
PFnGetIfTable            GetIfTable            = NULL; 
PFnGetIpAddrTable        GetIpAddrTable        = NULL;

//===================================================================================
static BOOL LoadIPHLPAPI(BOOL bLoad)
{
    static HMODULE hIPHLPAPI;

    if (bLoad) {

        if (hIPHLPAPI)
            return(TRUE);
        /*
         *  Get handle to iphlpapi.DLL
         */
        if ((hIPHLPAPI = LoadLibrary(TEXT("iphlpapi"))) == NULL )
            return(FALSE);

        /*
         *  Get entry points 
         */

        GetIpAddrTable        = (PFnGetIpAddrTable)        GetProcAddress( hIPHLPAPI, TEXT("GetIpAddrTable")); 
        GetNumberOfInterfaces = (PFnGetNumberOfInterfaces) GetProcAddress( hIPHLPAPI, TEXT("GetNumberOfInterfaces")); 
        GetFriendlyIfIndex    = (PFnGetFriendlyIfIndex   ) GetProcAddress( hIPHLPAPI, TEXT("GetFriendlyIfIndex"));
        GetIfEntry            = (PFnGetIfEntry           ) GetProcAddress( hIPHLPAPI, TEXT("GetIfEntry"));
        GetIfTable            = (PFnGetIfTable           ) GetProcAddress( hIPHLPAPI, TEXT("GetIfTable"));
    } else {
        FreeLibrary(hIPHLPAPI);
        hIPHLPAPI = NULL;
    }
    return(TRUE);
}

//===================================================================
void PrintLastError(char * msg = NULL)
{
    LPTSTR lpMessageBuffer;

    FormatMessage(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                 (LPTSTR) &lpMessageBuffer,
                 0,
                 NULL );

    //... now display this string
    if (msg) {
        fprintf(stdout,"Error> %s %d - %s",msg,GetLastError(),lpMessageBuffer); 
    } else {
        fprintf(stdout,"Error> %d - %s",GetLastError(),lpMessageBuffer); 
    }

    // Free the buffer allocated by the system

    LocalFree( (LPVOID) lpMessageBuffer );

}

//===================================================================================
void DisplayErrorText(DWORD dwLastError)
{
    HMODULE hModule = NULL; // default to system source
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_FROM_SYSTEM ;

    //
    // if dwLastError is in the network range, load the message source
    //

    if (dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
        hModule = LoadLibraryEx( TEXT("netmsg.dll"),NULL,LOAD_LIBRARY_AS_DATAFILE);

        if (hModule != NULL)
            dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    //
    // call FormatMessage() to allow for message text to be acquired
    // from the system or the supplied module handle
    //

    if (dwBufferLength = FormatMessageA(
                                       dwFormatFlags,
                                       hModule, // module to get message from (NULL == system)
                                       dwLastError,
                                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
                                       (LPSTR) &MessageBuffer,
                                       0,
                                       NULL
                                       )) {
        DWORD dwBytesWritten;

        //
        // Output message string on stderr
        //
        WriteFile(
                 GetStdHandle(STD_ERROR_HANDLE),
                 MessageBuffer,
                 dwBufferLength,
                 &dwBytesWritten,
                 NULL
                 );

        //
        // free the buffer allocated by the system
        //
        LocalFree(MessageBuffer);
    }

    //
    // if we loaded a message source, unload it
    //
    if (hModule != NULL)
        FreeLibrary(hModule);
}

//===================================================================================
LPSTR ToANSI(LPCWSTR wstr)
{
    static CHAR str[256];

    WideCharToMultiByte( CP_ACP, 0, wstr, -1, str, 256, NULL, NULL );

    return str;
}


//===================================================================================
void PrintType(DWORD dwType)
{
    switch (dwType) {
    case MIB_IF_TYPE_OTHER    :
        _tprintf( TEXT("other    "));
        break;
    case MIB_IF_TYPE_ETHERNET :
        _tprintf( TEXT("Ethernet "));
        break;
    case MIB_IF_TYPE_TOKENRING:
        _tprintf( TEXT("TokenRing"));
        break;
    case MIB_IF_TYPE_FDDI     :
        _tprintf( TEXT("FDDI     "));
        break;
    case MIB_IF_TYPE_PPP      :
        _tprintf( TEXT("PPP      "));
        break;
    case MIB_IF_TYPE_LOOPBACK :
        _tprintf( TEXT("LoopBack "));
        break;
    case MIB_IF_TYPE_SLIP     :
        _tprintf( TEXT("SLIP     "));
        break;
    default:
        _tprintf( TEXT("unknown  "));
    }
}

//===================================================================================
void PrintOperStat(DWORD dwStat)
{
    switch (dwStat) {
    case MIB_IF_OPER_STATUS_NON_OPERATIONAL :
        _tprintf( TEXT("Non operational"));
        break;
    case MIB_IF_OPER_STATUS_UNREACHABLE :
        _tprintf( TEXT("Unreachable"));
        break;
    case MIB_IF_OPER_STATUS_DISCONNECTED:
        _tprintf( TEXT("Disconnected"));
        break;
    case MIB_IF_OPER_STATUS_CONNECTING     :
        _tprintf( TEXT("Connecting"));
        break;
    case MIB_IF_OPER_STATUS_CONNECTED      :
        _tprintf( TEXT("Connected"));
        break;
    case MIB_IF_OPER_STATUS_OPERATIONAL     :
        _tprintf( TEXT("Operational"));
        break;
    default:
        _tprintf( TEXT("unknown"));
    }
}

//===================================================================================
int GetFirstMACaddr(unsigned char * pmac, unsigned int len,unsigned char * pmacold)
{
    static MIB_IFROW IfRowArray[200];  // 200 interfaces should be enough 
    static BOOL bNotFirstRun;
    ULONG dwSize = 0;
    ULONG dwInterfaces = 0;

    if (::GetNumberOfInterfaces(&dwInterfaces) != NO_ERROR) {
        printf("[E]  GetNumberOfInterfaces\n");
        return -1;
    }

    if (dwInterfaces == 0) {
        printf("This machine has 0 interfaces\n"); // should have at least the loopback
        return -1;
    }

    // lets find out the size of buffer we need
    GetIfTable(NULL,&dwSize,TRUE);
    PMIB_IFTABLE pIfTable = ( PMIB_IFTABLE )  new char [dwSize];
    if (::GetIfTable(pIfTable,&dwSize,TRUE) != NO_ERROR) {
        delete [] pIfTable;
        printf("[E] GetIfTable\n");
        return -1;                        
    }

    //_tprintf( TEXT("Size: %ld\n"), dwSize);

    //_tprintf( TEXT("IP entries: %ld\n\n"),pIpAddrTable->dwNumEntries);

    if (pmacold) {
        //printf("Old: %02X%02X%02X%02X%02X%02X\n",pmacold[0],pmacold[1],pmacold[2],pmacold[3],pmacold[4],pmacold[5]);
        for (unsigned int i = 0; i < pIfTable->dwNumEntries; i++ ) {

            PMIB_IFROW pIfRow = &pIfTable->table[i];

            if ((pIfRow->dwType == MIB_IF_TYPE_ETHERNET) ||
                (pIfRow->dwType == MIB_IF_TYPE_TOKENRING) || 
                (pIfRow->dwType == MIB_IF_TYPE_FDDI) ) {
                bool bMatch = true;
                for (unsigned int k = 0; k < pIfRow->dwPhysAddrLen; k++) {
                    if (k < len) {
                        if (pIfRow->bPhysAddr[k] != pmacold[k]) {
                            bMatch = false;
                            break;
                        }
                    } else 
                        break;
                }
                if (bMatch) {
                    for (unsigned int k = 0; k < len; k++) {
                        pmac[k] = pmacold[k];
                    }
                    delete [] pIfTable;
                    //printf("Unsing old Machineid\n");
                    return 0;
                }
            }
        }
    }

    for (unsigned int i = 0; i < pIfTable->dwNumEntries; i++ ) {

        PMIB_IFROW pIfRow = &pIfTable->table[i];

        if (g_bNOLOOPBACK && (pIfRow->dwIndex == 1)) 
            continue;

        //_tprintf( TEXT("\n==========================================================\n"));
        //_tprintf( TEXT("Table index: 0x%08X (full)  0x%08X (friendly)\n"),pIfRow->dwIndex,GetFriendlyIfIndex(pIfRow->dwIndex));
        //_tprintf( TEXT("Interface: %s \n"), pIfRow->bDescr );
        //_tprintf( TEXT("IfName:    %s \n"), ToANSI(pIfRow->wszName));
        //_tprintf( TEXT("Type:   "));
        //PrintType(pIfRow->dwType);
        //_tprintf( TEXT("  Status(A,O): %lu, %lu ("), pIfRow->dwAdminStatus,pIfRow->dwOperStatus);
        //PrintOperStat(pIfRow->dwOperStatus);
        //_tprintf( TEXT(")\n"));

        //_tprintf( TEXT("Mtu:    %-7ld          Speed: %lu  (%lu KB/s)\n"),
        //          pIfRow->dwMtu,pIfRow->dwSpeed,pIfRow->dwSpeed/8/1024);

        if (strstr((char *)pIfRow->bDescr,"PPP")) {
            continue;
        }

        //_tprintf( TEXT("MACadd: "));
        if ((pIfRow->dwType == MIB_IF_TYPE_ETHERNET) ||
            (pIfRow->dwType == MIB_IF_TYPE_TOKENRING) || 
            (pIfRow->dwType == MIB_IF_TYPE_FDDI) ) {
            for (unsigned int k = 0; k < pIfRow->dwPhysAddrLen; k++) {
                //_tprintf(TEXT("%02X"),pIfRow->bPhysAddr[k]);
                if (k < len) {
                    //sprintf(&pmac[k*2],"%02X",pIfRow->bPhysAddr[k] ^ 0xFF);
                    //pmac[k] = pIfRow->bPhysAddr[k] ^ 0xFF;
                    pmac[k] = pIfRow->bPhysAddr[k];
                } else 
                    break;
            }
            delete [] pIfTable;
            return 0;
        }
    }
    delete [] pIfTable;
    return -1;
}

//===================================================================================
static DWORD GetWinVolumeSerialNumber()
{
    TCHAR szWinDir[MAX_PATH+1];

    if (::GetWindowsDirectory(szWinDir,MAX_PATH+1) == 0) {
        //LogPrintf("[E] (MACHID) GetWindowsDirectory error");
        return 0;
    }

    szWinDir[3] = 0;
    DWORD dwVolumeSerialNumber;
    DWORD dwMaximumComponentLength;
    DWORD dwFileSystemFlags;
    if (::GetVolumeInformation(szWinDir,NULL,0,&dwVolumeSerialNumber,&dwMaximumComponentLength,&dwFileSystemFlags,NULL,0) == 0) {
        //LogPrintf("[E] (MACHID) GetVolumeInformation error");
        return 0;
    }

    //if (g_bDEBUG) 
    //   _tprintf( TEXT("Volume serial: %lu - 0x%X \n"),dwVolumeSerialNumber,dwVolumeSerialNumber);

    return dwVolumeSerialNumber;
}

//============================================================
#define SETUPKEY       "SOFTWARE\\Automatos\\"
#define KEY_COMMON     "Asa\\Common"
#define KEY_VER_COM    "\\3.0"
static bool ReadRegistryMID(char * pmid, int len)
{
    TCHAR szKey[256];
    HKEY hKey = 0;
    DWORD dwType; 
    DWORD dwSize;
    LONG lRet = 0;
    bool bret = false;

    _tcscpy(szKey, SETUPKEY);
    _tcscat(szKey, KEY_COMMON);
    _tcscat(szKey, KEY_VER_COM);

    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          szKey,
                          0,
                          KEY_QUERY_VALUE,
                          &hKey);
    if (lRet != ERROR_SUCCESS) 
        return false;


    dwType = REG_BINARY;
    dwSize = len;
    lRet = ::RegQueryValueEx(hKey,
                             TEXT("MachineID"),
                             NULL,
                             &dwType,
                             (BYTE*)pmid,
                             &dwSize);
    if (lRet != ERROR_SUCCESS) goto cleanup;
    bret = true;

    cleanup:
    ::RegCloseKey(hKey);

    return bret;
}

//============================================================
int Get_MachineID(unsigned char * mid)
{
    int iret = -1;
    static unsigned char machid[12];

    if (machid[10]) {
       for (int i = 0; i < 12; i++) {
           mid[i] = machid[i];
       }
    }

    if (LoadIPHLPAPI(TRUE)) {
        unsigned char midold[12];
        if (ReadRegistryMID((char *)midold,12)) {
            //printf("Got old Machineid\n");
            iret = GetFirstMACaddr(&machid[0],6,midold);
        } else {
            iret = GetFirstMACaddr(&machid[0],6,NULL);
        }
    }
    LoadIPHLPAPI(FALSE);   

    DWORD volnum = GetWinVolumeSerialNumber();
    machid[6] = (unsigned char) (volnum >> 24) ;
    machid[7] = (unsigned char) (volnum >> 16) ;
    machid[8] = (unsigned char) (volnum >> 8)  ;
    machid[9] = (unsigned char)  volnum ;

    DWORD dwVersion = GetVersion();
    if (dwVersion < 0x80000000)                // Windows NT/2000
        machid[10] = (M_WINNT << 4) + M_INTELX86;
    else
        machid[10] = (M_WIN9X << 4) + M_INTELX86;

    machid[11] = Calc_MachineID_CD(machid);

    //printf("\n%08X\n",volnum);
    for (int i = 0; i < 12; i++) {
        mid[i] = machid[i];
        //printf("%02X",mid[i]);
    }
    //printf("\n");

    if ( (iret != 0) || (volnum == 0)) {
        return -1;
    }

    return 0;
}

//cl dumpinterfaces.cpp ws2_32.lib advapi32.lib 




