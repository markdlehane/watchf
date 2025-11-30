/**
 * @file main.c
 * @author Mark Lehane (markdlehane@gmail.com)
 * @brief Program entry point.
 * @details This file enables configuration through command 
 * line options and configuration files.
 * @version 0.1
 * @date 2025-11-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <getopt.h>

/* Build number data. */
static const unsigned VERSION_NO = 0;
static const char *BUILD_NO = "0.138";
static const char *APP_TITLE = "File/Directory Watcher";
static const char *APP_HELP = "Watches for a change on a file, files or directory,\nthen executes the given command";

/**
 * @brief Command line options container structure.
 * 
 */
typedef struct prog_options_s {
    bool verbose    : 1;
    bool trace      : 1;
    bool dump       : 1;
    bool monitor    : 1;

} prog_opts;

typedef enum opt_idents_e {
    OID_HELP = 0,
    OID_VERSION,
    OID_PATH,
    OID_FILE,
    OID_STDIN,
    OID_EXEC,
    OID_ONCE,
    OID_VERBOSE,
    OID_END

} opt_idents_t;

static const char *s_short_opts = "hf:se:1v";

static struct option s_long_options[] = {
    { "help",       no_argument,        NULL,   '?' },
    { "version",    no_argument,        NULL,   0   },
    { "path",       no_argument,        NULL,   0   },
    { "file",       required_argument,  NULL,   'f' },
    { "stdin",      no_argument,        NULL,   's' },
    { "exec",       required_argument,  NULL,   'e' },
    { "once",       no_argument,        NULL,   '1' },
    { "verbose",    no_argument,        NULL,   'v' },
    { NULL,         no_argument,        NULL,   0   } 
};

static const char *s_option_help[] = {
    "-?,-h,--help   displays this message.",
    "--version      displays the version and build number of this program.",
    "--path         displays the program path on stdout.",
    "--file,-f      activates the monitor unit.",
    "--stdin,-s     read stdin rather than a file.",
    "--exec,-e      program to execute upon a change event",
    "--once,-1      waits for a single change, default is a continuous scan.",
    "--verbose,-v   prints debug information and event data to stdout",
    NULL
};

/* Filename pointer for watched file. */
static const char *s_watch_target = NULL;
static const char *s_exec_command = NULL;
static bool s_watch_stdin = false;
static bool s_continuous = true;
static bool s_verbose = false;

/**
 * @brief Watch a file, files or a directory for changes.
 * @details The routine wathes a file, files or directory for changes
 * and executes a command for each change.
 * 
 * @param file_pattern A pointer to the file pattern to scan for changes.
 * @param executable A pointer to the command to execute.
 * @param continuous If true the program scans continuously, otherwise scans once.
 * @param verbose Report events and debug information.
 * @return int 0 on success.
 */
extern int watch_for_changes(const char *file_pattern, const char *executable, bool continuous, bool verbose);

/**
 * @brief Report the program path on stdout.
 * 
 */
static void report_path(const char *title) {
    char path[256];
    getcwd(path, sizeof(path));
    printf("%s: %s\n", title, path);
}

/**
 * @brief Report the version and build number data.
 * 
 */
static void report_version(void) {
    printf("Version %u:%s\n", VERSION_NO, BUILD_NO);
}

/**
 * @brief Report the available command line parmeters.
 * 
 */
static void report_help(void) {
    report_path(APP_TITLE);
    puts(APP_HELP);
    int help = 0;
    while (s_option_help[help] != NULL) {
        printf("\t%s\n", s_option_help[help]);
        help++;
    }
    puts("");
}

/**
 * @brief Program entry point.
 * 
 * @param argc The number of arguments.
 * @param argv Array of arguments.
 * @returns int 0 - normal exit.
 * @returns int -ve on errors.
 */
int main(int argc, char **argv) {
    char ch;
    int c;
    int digit_optind = 0;
    prog_opts optvals = {0};
    bool gamedev_copy = false;

    int ret = EXIT_FAILURE;
    if (argc == 1) {
        report_help();
    }
    else {
        bool run = true;
        while (run) {
            int this_option_optind = optind ? optind : 1;
            int option_index = 0;

            c = getopt_long(argc, argv, s_short_opts, s_long_options, &option_index);
            if (c == -1) {
                break;
            }
            else {
                // Handle short option codes.
                if (c == '?' || c == 'h') {
                    option_index = OID_HELP;
                }
                else if (c == 'f') {
                    option_index = OID_FILE;
                }
                else if (c == 's') {
                    option_index = OID_STDIN;
                }
                else if (c == 'e') {
                    option_index = OID_EXEC;
                }
                else if (c == '1') {
                    option_index = OID_ONCE;
                }
                else if (c == 'v') {
                    option_index = OID_VERBOSE;
                }

                // Process the selected option.
                switch(option_index) {
                    case OID_HELP:
                        report_help();
                        run = false;
                        break;
                    case OID_VERSION:
                        report_version();
                        run = false;
                        break;
                    case OID_PATH:
                        report_path(APP_TITLE);
                        run = false;
                        break;
                    case OID_FILE:
                        s_watch_target = optarg;
                        s_watch_stdin = false;
                        break;
                    case OID_STDIN:
                        s_watch_target = NULL;
                        s_watch_stdin = true;
                        break;
                    case OID_EXEC:
                        s_exec_command = optarg;
                        break;
                    case OID_ONCE:
                        s_continuous = false;
                        break;
                    case OID_VERBOSE:
                        s_verbose = true;
                        break;
                    default:
                        printf("?? getopt returned character code 0%o ??\n", c);
                        run = false;
                        break;
                }
            }
        }
        // Report the operation mode.
        if (run) {
            if (s_watch_target == NULL) {
                puts("Please supply a filename pattern to watch for changes.");
            }
            else if (s_exec_command == NULL) {
                puts("Please supply a command to execute on file chaneg.");
            }
            else {
                if (s_verbose) {
                    printf("%s watch for change on %s\n", s_continuous ? "Continuous" : "Single", s_watch_target);
                    printf("Execute '%s' on event.\n", s_exec_command);
                }
                ret = watch_for_changes(s_watch_target, s_exec_command, s_continuous, s_verbose);
            }
        }
    }
    return ret;
}

/* End. */
