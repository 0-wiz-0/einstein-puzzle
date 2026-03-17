#ifdef WIN32
#include <windows.h>

#include <processenv.h>
#include <shellapi.h>

PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc);
#endif
