#include <stdlib.h>
#ifndef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
#include <unistd.h>
#else
#include <io.h>
#endif
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

char directory_getcurrent()
{
    #define bufsize 256
    char* buf = malloc(bufsize);
    getcwd(buf, bufsize);
    return buf;
}

int printdir (char *dirname)
{
    DIR *FD;
    struct dirent *f;

    if (NULL == (FD = opendir (dirname)))
    {
        printf ("Unable to load directory.\n");
        return (-1);
    }
    printf ("%s :\n", dirname);
    while ((f = readdir (FD)))
    {
        printf ("    %s\n", f->d_name);
    }
    closedir (FD);
    return (0);
}

void
print_fileinfo (char *filename)
{
    #ifndef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
    #include <pwd.h>
    #include <grp.h>
    struct stat buf;
    struct passwd *pwd;
    struct group *grp;

    if (stat (filename, &buf))
    {
        fprintf (stderr, "Unable to locate file %s or unable to get it's stats.\n", filename);
        return;
    }

    /* Type de fichier */
    if (S_ISREG (buf.st_mode)) printf ("Regular file.\n");
    else if (S_ISDIR (buf.st_mode)) printf ("Directory\n");
    else if (S_ISCHR (buf.st_mode))
        printf ("Character device\n");
    else if (S_ISBLK (buf.st_mode))
        printf ("Bloc device\n");
    else if (S_ISFIFO (buf.st_mode)) printf ("FIFO\n");
    else if (S_ISLNK (buf.st_mode)) printf ("Symbolic link\n");
    else if (S_ISSOCK (buf.st_mode)) printf ("Socket\n");
    else printf ("Unknow type.\n");

    /* UID, GID */
    pwd = getpwuid (buf.st_uid);
    grp = getgrgid (buf.st_gid);
    printf ("Owner : nom='%s' groupe='%s'\n",
            pwd ? pwd->pw_name : "?",
            grp ? grp->gr_name : "?");

    /* Taille */
    printf ("Size : %ld\n", buf.st_size);

    /* Heure de dernière modification */
    printf ("Last Modified : %s\n", ctime (&buf.st_mtime));
    #else
    printf("Sorry, this function is available only on UNIX.");
    #endif
}

