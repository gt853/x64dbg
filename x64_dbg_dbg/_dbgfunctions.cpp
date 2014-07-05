#include "_global.h"
#include "_dbgfunctions.h"
#include "assemble.h"
#include "debugger.h"
#include "addrinfo.h"
#include "patches.h"
#include "memory.h"

static DBGFUNCTIONS _dbgfunctions;

const DBGFUNCTIONS* dbgfunctionsget()
{
    return &_dbgfunctions;
}

static bool _sectionfromaddr(duint addr, char* section)
{
    HMODULE hMod=(HMODULE)modbasefromaddr(addr);
    if(!hMod)
        return false;
    char curModPath[MAX_PATH]="";
    if(!GetModuleFileNameExA(fdProcessInfo->hProcess, hMod, curModPath, MAX_PATH))
        return false;
    HANDLE FileHandle;
    DWORD LoadedSize;
    HANDLE FileMap;
    ULONG_PTR FileMapVA;
    if(StaticFileLoad(curModPath, UE_ACCESS_READ, false, &FileHandle, &LoadedSize, &FileMap, &FileMapVA))
    {
        uint rva=addr-(uint)hMod;
        int sectionNumber=GetPE32SectionNumberFromVA(FileMapVA, GetPE32DataFromMappedFile(FileMapVA, 0, UE_IMAGEBASE)+rva);
        if(sectionNumber>=0)
        {
            const char* name=(const char*)GetPE32DataFromMappedFile(FileMapVA, sectionNumber, UE_SECTIONNAME);
            if(section)
                strcpy(section, name);
            StaticFileUnload(curModPath, false, FileHandle, LoadedSize, FileMap, FileMapVA);
            return true;
        }
        StaticFileUnload(curModPath, false, FileHandle, LoadedSize, FileMap, FileMapVA);
    }
    return false;
}

static bool _patchget(duint addr)
{
    return patchget(addr, 0);
}

static bool _patchinrange(duint start, duint end)
{
    if(start > end)
    {
        duint a=start;
        start=end;
        end=a;
    }
    for(duint i=start; i<end+1; i++)
        if(_patchget(i))
            return true;
    return false;
}

static bool _mempatch(duint va, const unsigned char* src, duint size)
{
    return mempatch(fdProcessInfo->hProcess, (void*)va, src, size, 0);
}

static void _patchrestorerange(duint start, duint end)
{
    if(start > end)
    {
        duint a=start;
        start=end;
        end=a;
    }
    for(duint i=start; i<end+1; i++)
        patchdel(i, true);
}

void dbgfunctionsinit()
{
    _dbgfunctions.AssembleAtEx=assembleat;
    _dbgfunctions.SectionFromAddr=_sectionfromaddr;
    _dbgfunctions.ModNameFromAddr=modnamefromaddr;
    _dbgfunctions.ModBaseFromAddr=modbasefromaddr;
    _dbgfunctions.ModBaseFromName=modbasefromname;
    _dbgfunctions.ModSizeFromAddr=modsizefromaddr;
    _dbgfunctions.Assemble=assemble;
    _dbgfunctions.PatchGet=_patchget;
    _dbgfunctions.PatchInRange=_patchinrange;
    _dbgfunctions.MemPatch=_mempatch;
    _dbgfunctions.PatchRestoreRange=_patchrestorerange;
}