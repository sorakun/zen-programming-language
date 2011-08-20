/*
 * The Zen console for Windows/Unix.
 * See Copyright Notice in zen.h.
*/

#ifdef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
    #include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include "zen.h"
#include "autil.h"
#include "aconsole.h"
#include "alib.h"

void zen_prompt(int argc, char *argv[])
{
    zen_openlibs();
    #ifdef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
        HRSRC hResource;
        HGLOBAL hResourceLoaded;
        HMODULE hLibrary;
    #endif
    char* lpBuffer;
    char buffer[10240] = {0};
    size_t size;
    char* pos;
    int x;
    if (argc <= 1)
    {
        #ifdef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
        hLibrary = GetModuleHandle(NULL);
        hResource = FindResource(hLibrary,
                                 MAKEINTRESOURCE(ZEN_START_RESOURCE_INDEX), RT_RCDATA);
        if (NULL != hResource)	/* stand-alone executable */
        {
            hResourceLoaded = LoadResource(hLibrary, hResource);
            if (NULL != hResourceLoaded)
            {
                lpBuffer = (char*)LockResource(hResourceLoaded);
                if (NULL != lpBuffer)
                {
                    size = SizeofResource(hLibrary, hResource);
                    memcpy(buffer, lpBuffer, size);
                    zen_runstring("(msg)", buffer);
                }
            }
        }
        else	/* interactive mode */
        #endif
        {
            printf(ZEN_CON_HEADER, ZEN_VERSION, system_getname(0));
            int keep = 1;
            while (keep)
            {
                #ifndef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
                    #include <sys/utsname.h>
                    struct utsname name;
                    uname(&name);
                    printf(CON_CURSOR, getenv("USER"), name.nodename);
                #endif
                fgets(buffer, 1024, stdin);
                strcut(buffer, " \t\n\r");
                // sys cmds
                if ((pos=strstr(buffer, ".run")) == buffer)
                {
                    pos += strlen(".run")+1;
                    zen_runfile(pos);
                }
                else if ((pos=strstr(buffer, ".debug")) == buffer)
                {
                    pos += strlen(".debug")+1;
                    zen_debugfile(pos);
                }

                else if (strcmp(buffer, ".exit") == 0)
                    keep = 0;

                else if (strcmp(buffer, ".help") == 0)
                    printf(ZEN_CON_HELP);

                else if (strcmp(buffer, ".license") == 0)
				    printf(ZEN_CON_LICENSE);
/*
                else if (strcmp(buffer, "list all") == 0)
                    printdir(directory_getcurrent());

                else if ((pos=strstr(buffer, "cd")) == buffer)
                {
                    pos += strlen("cd")+1;
                    set_curdir(pos);
                }

                else if ((pos=strstr(buffer, "makedir")) == buffer)
                {
                    pos += strlen("makedir")+1;
                    if(!make_dir(pos))
                       printf("Unable to create directory.\n");
                }

                else if ((pos=strstr(buffer, "remdir")) == buffer)
                {
                    pos += strlen("remdir")+1;
                    if(!remove_dir(pos))
                       printf("Unable to remove directory.\n");
                }

                else if (strcmp(buffer, "dir usage") == 0)
                {
                    if(! (x = dir_usage(get_curdir(), 0)))
                        printf("Unable to get directory's usage.\n");
                    else
                        printf("%d\n", x);

                }

                else if (strcmp(buffer, "dir usage all") == 0)
                {
                    if(! (x = dir_usage(get_curdir(), 1)))
                        printf("Unable to get directory's usage.\n");
                    else
                        printf("%d\n", x);
                }

                else if (strcmp(buffer, "dir files") == 0)
                {
                    if(! (x = dir_files(get_curdir(),  0)))
                        printf("Unable to get directory's files count.\n");
                    else
                        printf("%d\n", x);
                }

                else if (strcmp(buffer, "dir files all") == 0)
                {
                    if(! (x = dir_files(get_curdir(), 1)))
                        printf("Unable to get directory's files count.\n");
                    else
                        printf("%d\n", x);
                }

                else if ((pos=strstr(buffer, "file info")) == buffer)
                {
                    pos += strlen("file info")+1;
                    print_fileinfo(pos);
                }
*/
                else if (strlen(buffer) > 0)
                    zen_runstring("(CONSOLE)", buffer);
                printf("\n");
            }
        }
        #ifdef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
        FreeLibrary(hLibrary);
        #endif
    }
    else	/* command-line mode */
        zen_runfile(argv[1]);
    zen_closelibs();
}
