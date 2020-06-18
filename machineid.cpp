#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdafx.h"
#include "machineid.h"

//============================================================
int Get_MachineID_asString(char * mid)
{
    unsigned char machid[12];
    int iret = Get_MachineID(machid);

    for (int i = 0; i < 12; i++) {
        sprintf(&mid[i*2],"%02X",machid[i]);
    }

    return iret;
}

//============================================================
char Calc_MachineID_CD(unsigned char * array)
{
    long dv = 101;
    int i;

    for (i = 0; i < 11; i++)
        dv += (i+1) * array[i];

    dv %= 100;

    return (char) dv;
}

//============================================================
int Check_MachineID_CD(unsigned char * array)
{
    char dv = Calc_MachineID_CD(array);

    return (dv == array[11]) ? 0 : -1;
}

//============================================================
static int ctoi(char c)
{
    if ( (c >= '0') && (c <= '9') )
        return c - '0';

    if ( (c >= 'a') && (c <= 'f') )
        return c - 'a' + 0x0A;

    if ( (c >= 'A') && (c <= 'F') )
        return c - 'A' + 0x0A;

    return 0;
}

//============================================================
int Check_MachineID_CD_fromString(char * str)
{
    unsigned char machid[12];

    char * ptr = str;
    for (int i = 0; i < 12; i++) {
        machid[i] = (ctoi(*ptr) << 4) + ctoi(*(ptr+1));
        ptr += 2;
    }

    return Check_MachineID_CD(machid);
}


