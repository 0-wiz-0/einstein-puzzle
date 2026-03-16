#ifdef WIN32
#include <processenv.h>
#include <shellapi.h>
#include <windows.h>

PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc);
#endif
