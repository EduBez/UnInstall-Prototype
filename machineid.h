#ifndef _machineid_h
	#define _machineid_h

#define _NO_DLL

#ifndef _AUC_EXPORT
#if defined(_WIN32) && !defined(_NO_DLL)
#   ifdef _BUILD_DLL
#      define _AUC_EXPORT __declspec(dllexport)
#   else
#      define _AUC_EXPORT __declspec(dllimport)
#   endif
#else
#   define _AUC_EXPORT
#endif
#endif

// OS identifier
#define M_WINNT    1
#define M_WIN9X    2
#define M_AIX      3
#define M_SOLARIS  4
#define M_LINUX    5
#define M_HPUX     6
#define M_TRU64    7

// Platform identifier
#define M_INTELX86 1
#define M_INTELI64 2
#define M_RISC     3
#define M_IBMS390  4
#define M_ALPHA    5

// MachineID is 12 bytes long with the following structure:
// bytes 0 - 5 -> MAC addr XORED with 0xFF
// bytes 6 - 9 -> platform specific
// byte 10     -> high nible - OS, low nible - platform
// byte 11     -> modulus 100 check digit

#ifdef __cplusplus
extern "C" {
#endif

_AUC_EXPORT  char   Calc_MachineID_CD(unsigned char  array []);  
_AUC_EXPORT  int    Check_MachineID_CD(unsigned char  array []);      // returns 0 if ok, -1 if error
_AUC_EXPORT  int    Check_MachineID_CD_fromString(char * str);     // returns 0 if ok, -1 if error
_AUC_EXPORT  int    Get_MachineID(unsigned char array []);        // returns -1 if machid incomplete
_AUC_EXPORT  int    Get_MachineID_asString(char * str);  // returns -1 if machid incomplete

#ifdef __cplusplus
}
#endif

#endif
