/*-----------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Loader for ADOM and for the frontend
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <cstring>
using namespace std;

char *optarg;
int optind, opterr, optopt;

const char *usage =
    "Usage: adom-sage [OPTION]... -- [PARAMETERS TO ADOM]...\n"
    "  -a  --adom-path=PATH  Path to adom binary.\n"
    "  -d  --debug-level=x   Debug level (a number).\n"
    "  -h, --help            Print this screen.\n"
    "  -l, --logfile=PATH    Path to log file (only used if nonzero debug level).\n"
    "  -p, --passive         Run passively (logs without modifying gameplay).\n"
    "  -s, --sage-path=PATH  Path to adom-sage.so.\n"
    "  -v, --version         Display version info.\n"
    "  --                    End of options to Sage; further options are for ADOM";

const char *version =
    "ADOM Sage version " "0.9.18" " - frontend for ADOM\n"
    "ADOM is (C) Copyright 1994-2013 Thomas Biskup.\n"
    "ADOM Sage is Copyright (c) 2002 Joshua Kelley.\n"
    "ADOM Sage is Copyright (c) 2012-2013 Christopher Henning.\n";

struct option longopts[] =
{
    {"passive",     0, NULL, 'p'},
    {"help",        0, NULL, 'h'},
    {"version",     0, NULL, 'v'},
    {"logfile",     1, NULL, 'l'},
    {"debug-level", 1, NULL, 'd'},
    {"adom-path",   1, NULL, 'a'},
    {"sage-path",   1, NULL, 's'},
    {0, 0, 0, 0}
};

// Searches through path (a colon-separated list of directories) for filename.
// Destroys path in the process.  Returns the directory containing filename.
// Requiring a dynamically-allocated, hard-to-free path parameter is sloppy,
// but we don't run for long enough to care.
char *search_path (const char *filename, char *path)
{
    string full_path;
    struct stat buf;
    char *s;

    while ((s = strchr(path, ':')) != NULL)
    {
        *s = '\0';
        full_path = path;
        full_path += "/";
        full_path += filename;

        if (stat(full_path.c_str(), &buf) == 0)
        {
            return path;
        }

        path = s + 1;
    }

    full_path = path;
    full_path += "/";
    full_path += filename;

    if (stat(full_path.c_str(), &buf) == 0)
    {
        return path;
    }

    return NULL;
}

// Tries to locate filename in one of four locations:
//   1. Current contents of file_path (if not empty)
//   2. The base_path
//   3. One of the directories from the colon-separated list in search_dirs
//   4. Bare filename (last-resort approach)
// file_path contains the result on exit
void locate_file(const char *filename, string &file_path, string &base_path,
                 const char *search_dirs)
{
    if (file_path.empty())
    {
        if (!base_path.empty())
        {
            file_path = base_path + filename;
        }

        else
        {
            // Search through PATH to find file if necessary
            char *work_path = NULL;
            char *search_copy = strdup(search_dirs);

            if (search_path != NULL)
            {
                work_path = search_path(filename, search_copy);
            }

            if (work_path != NULL)
            {
                file_path = work_path;
                file_path += "/";
                file_path += filename;
            }

            else
            {
                file_path = filename;
            }

            free(search_copy);
        }
    }
}

int main (int argc, char *argv[])
{
    int result, i;
    string base_path, sage_path, adom_path, libc_path, curses_path;
    char *env_path;
    char **adom_argv;

    // Process options
    opterr = 1;

    while (1)
    {
        result = getopt_long(argc, argv, "a:d:l:hps:v", longopts, NULL);

        if (result == -1)
        {
            break;
        }

        switch (result)
        {

            case 'a':    // Adom path
                adom_path = optarg;
                break;

            case 'd':    // Debug level
                result = setenv("SAGE_DEBUG", optarg, 1);
                break;

            case 'h':    // Help
                puts(usage);
                exit(0);
                break;

            case 'l':    // Log path
                result = setenv("SAGE_LOGFILE", optarg, 1);
                break;

            case 'p':    // Passive option
                result = setenv("SAGE_PASSIVE", "1", 1);
                break;

            case 's':    // Sage path
                sage_path = optarg;
                break;

            case 'v':    // Version
                puts(version);
                exit(0);

            default:
                puts(usage);
                exit(1);
                break;
        }

        if (result == -1)
        {
            printf("adom-sage: Unable to set config variables\n");
            exit(1);
        }

    }

    // Determine base path of this program
    if (strchr(argv[0], '/'))
    {
        base_path = dirname(argv[0]);    // NOTE this may modify argv[0]
        base_path += "/";
    }

    else
    {
        base_path = "";
    }

    env_path = getenv("PATH");

    locate_file("adom", adom_path, base_path, env_path);
    locate_file("adom-sage.so", sage_path, base_path, env_path);

    // Determine paths to shared libraries
    if (libc_path.empty() || curses_path.empty())
    {
        FILE *stream;
        string cmdline;
        char buffer[255], *library, *arrow, *libpath;

        cmdline = "ldd ";
        cmdline += adom_path;
        stream = popen(cmdline.c_str(), "r");

        if (stream != NULL)
        {
            while (!feof(stream))
            {
                if (fgets(buffer, sizeof(buffer), stream) == NULL)
                {
                    break;
                }

                library = strtok(buffer, " \t");

                if (library == NULL)
                {
                    continue;
                }

                arrow = strtok(NULL, " \t");

                if ((arrow == NULL) || (strcmp(arrow, "=>") != 0))
                {
                    continue;
                }

                libpath = strtok(NULL, " \t");

                if (libpath == NULL)
                {
                    continue;
                }

                if (strcmp(library, "libncurses.so.5") == 0)
                {
                    curses_path = libpath;
                }

                if (strcmp(library, "libc.so.6") == 0)
                {
                    libc_path = libpath;
                }
            }

            pclose(stream);
        }
    }

    if (curses_path.empty() || libc_path.empty())
    {
        printf("ADOM Sage: Can't get library info from ADOM, using compile-time defaults.\n");
        unsetenv("CURSES_PATH");
        unsetenv("LIBC_PATH");
    }

    else
    {
        result = setenv("CURSES_PATH", curses_path.c_str(), 1);

        if (result != -1)
        {
            result = setenv("LIBC_PATH", libc_path.c_str(), 1);
        }

        if (result == -1)
        {
            printf("adom-sage: Unable to initialize library information\n");
            exit(1);
        }
    }

    // Prepare to load library
    char *preload = getenv("LD_PRELOAD");

    if (preload == NULL)
    {
        result = setenv("LD_PRELOAD", sage_path.c_str(), 1);
    }

    else
    {
        string new_preload(preload);
        new_preload += ' ';
        new_preload += sage_path;
        result = setenv("LD_PRELOAD", new_preload.c_str(), 1);
    }

    if (result == -1)
    {
        printf("adom-sage: Unable to load ADOM Sage\n");
        exit(1);
    }

    // Try to get info on ADOM executable
    struct stat buf;
    result = stat(adom_path.c_str(), &buf);

    if (result != -1)      // Ignore errors (perhaps unwisely)
    {

        // Check if ADOM is setuid; warn if it is
        if (buf.st_mode & (S_ISUID | S_ISGID))
        {
            printf("WARNING: ADOM is installed setuid or setgid, so ADOM Sage\n");
            printf("probably won't work.\n\n");
            printf("[PRESS ENTER TO CONTINUE]");
            getchar();
        }

        // Try to determine ADOM's version while we're at it
        const char *version;

        switch (buf.st_size)
        {
            case 2388548:
                version = "100";
                break;
            case 2556118:
                version = "110";
                break;
            case 2452608:
                version = "111";
                break;
	          case 2240572: // 32-bit Debian
            		version = "1203"; 
            		break;
      	    case 2246972: // 32-bit Debian
            		version = "1204"; 
            		break;
            case 2250044: // 32-bit Debian
                version = "1205"; 
                break;
            case 2267132: // 32-bit Debian
                version = "1206";
                break;
            case 2301084: // 32-bit Debian
                version = "1207";
                break;
            case 2315196: // 32-bit Debian
                version = "1208";
                break;
            case 2383036: // 32-bit Debian
                version = "1209";
                break;
            case 2379196: // 32-bit Debian
                version = "12010";
                break;
            case 2378908: // 32-bit Debian
                version = "12011";
                break;
            case 2444220: // 32-bit Debian
                version = "12012";
                break;
            case 2444316: // 32-bit Debian
                version = "12013";
                break;
            case 2457248: // 32-bit Debian non-noteye
                version = "12014";
                break;
            case 2471612: // 32-bit Debian non-noteye
                version = "12016";
                break;
            case 2490700: // 32-bit Debian non-noteye
                version = "12017";
                break;
            case 2492396: // 32-bit Debian non-noteye
                version = "12018";
                break;
            case 2495376: // 32-bit Debian non-noteye
                version = "12019";
                break;
            case 2495248: // 32-bit Debian non-noteye
                version = "12020";
                break;
            default:
                version = "0";
                break;
        }

        setenv("ADOM_VERSION", version, 1);
    }

    // Run ADOM
    adom_argv = new char*[argc - optind + 2];
    adom_argv[0] = strdup(adom_path.c_str());

    for (i = 0; i <= argc - optind; i++)
    {
        adom_argv[1+i] = argv[optind+i];
    }

    adom_argv[i] = NULL;
    execvp(adom_path.c_str(), adom_argv);
    perror("adom-sage: Unable to run ADOM");
    exit(1);
}
