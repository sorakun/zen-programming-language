
#include <time.h>

char system_getname()
{
    #ifndef  __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
        #include <sys/utsname.h>
        struct utsname name;
        if(uname(&name)) exit(-1);
        return name.sysname;
    #elif __WIN32__
        return getenv("os");
    #endif
}

char system_date()
{
    time_t t;
    if(time(&t) != (time_t)-1)
       return ctime(&t);
    else
       return NULL;
}
