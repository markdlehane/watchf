/**
 * @file watch.c
 * @author Mark Lehane (markdlehane@gmail.com)
 * @brief Watch functions.
 * @details This module provides methods that watch a file/directory for changes
 * and invoke a user specified command to execute on change events.
 *
 * @version 0.1
 * @date 2025-11-27
 * 
 * @copyright Copyright (c) 2025, Mark D Lehane.
 * 
 */

#include <sys/signalfd.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

/**
 * @brief Enum used to identify event types.
 * 
 */
typedef enum poll_fd_e {
    FD_POLL_SIGNAL = 0,
    FD_POLL_INOTIFY,
    FD_POLL_MAX

} POLL_FDE;

// Local constants.
#define EVENT_BUF_LEN (sizeof(struct inotify_event) * 16)

// Local data.
static int s_watch_instance = -1;
static int s_inotify_instance = -1;

/**
 * @brief Report event types.
 * 
 * @param event A pointer to an event structure.
 */
static void report_event(struct inotify_event *event) {

    // Print generic header.
    printf("wd=%d mask=%08x cookie=%08x len=%d dir=%s",
        event->wd, event->mask, event->cookie, event->len,
        (event->mask & IN_ISDIR) ? "yes" : "no"
    );
    if (event->len) {
        printf(" name=%s", event->name);
    }
    printf(" ");

    // Report the event types.
    if (event->mask & IN_ACCESS) {
        printf ("IN_ACCESS,");
    }
    if (event->mask & IN_ATTRIB) {
        printf ("IN_ATTRIB,");
    }
    if (event->mask & IN_OPEN) {
        printf ("IN_OPEN,");
    }
    if (event->mask & IN_CLOSE_WRITE) {
        printf ("IN_CLOSE_WRITE,");
    }
    if (event->mask & IN_CLOSE_NOWRITE) {
        printf ("IN_CLOSE_NOWRITE,");
    }
    if (event->mask & IN_CREATE) {
        printf ("IN_CREATE,");
    }
    if (event->mask & IN_DELETE) {
        printf ("IN_DELETE,");
    }
    if (event->mask & IN_DELETE_SELF) {
        printf ("IN_DELETE_SELF,");
    }
    if (event->mask & IN_MODIFY) {
        printf ("IN_MODIFY,");
    }
    if (event->mask & IN_MOVE_SELF) {
        printf ("IN_MOVE_SELF,");
    }
    if (event->mask & IN_MOVED_FROM) {
        printf ("IN_MOVED_FROM (cookie: %d),", event->cookie);
    }
    if (event->mask & IN_MOVED_TO) {
        printf ("IN_MOVED_TO (cookie: %d)", event->cookie);
    }
    puts("");
    fflush (stdout);
}

/**
 * @brief Monitor a target for file/directory update events.
 * 
 * @param inf Inotiy interface handle.
 * @return int Return the number of "MODIFY" events that occurred.
 */
static int watch_handler(int inf, bool verbose) {
    char buf[EVENT_BUF_LEN] __attribute__((aligned(4)));
    ssize_t len = EVENT_BUF_LEN, i = 0;

    int events = 0;
    len = read(inf, buf, len);
    while (i < len) {
        struct inotify_event *event = (struct inotify_event*)&buf[i];

        // If a modify event occurs, add it to the "events" counter.
        if (event->mask & IN_MODIFY) {
            if (verbose) {
                report_event(event);
            }
            events++;
        }
        i += sizeof(struct inotify_event) + event->len;
    }
    // Return the number of "MODIFY" events that occurred.
    return events;
}

/**
 * @brief Initialise the watcher mechanism.
 * 
 * @param target The file or directory to watch.
 * @param verbose True if verbose output should be made.
 * @return int inotify handle.
 */
static int initialise_watcher(const char *target, const bool verbose) {
    // Create an inotify interface.
    int inf = inotify_init();
    if (inf == -1) {
        perror("Failed to initalise iNotify");
    }
    else {
        int wd = inotify_add_watch(inf, target, IN_MODIFY | IN_EXCL_UNLINK);
        if (wd == -1) {
            perror("Failed to create a watch on target");
        }
        else {
            if (verbose) {
                printf("Begun monitoring of '%s' - %d\n", target, wd);
            }
            s_inotify_instance = inf;
            s_watch_instance = wd;
        }
    }
    return inf;
}

/**
 * @brief Cleanly shutdown the inotify watcher instance.
 * 
 */
static void shutdown_watcher(void) {
    if (s_inotify_instance != -1) {
        if (s_watch_instance != -1) {
            inotify_rm_watch(s_inotify_instance, s_watch_instance);
            s_watch_instance = -1;
        }
        close(s_inotify_instance);
        s_inotify_instance = -1;
    }
}

/**
 * @brief Initialise the signals to report on.
 * 
 * @return int Signal interface handle.
 */
static int initialize_signals (void) {
    int signal_fd;
    sigset_t sigmask;

    /* We want to handle SIGINT and SIGTERM in the signal_fd, so we block them. */
    sigemptyset (&sigmask);
    sigaddset (&sigmask, SIGINT);
    sigaddset (&sigmask, SIGTERM);
    
    // Can we block the signals?
    if (sigprocmask (SIG_BLOCK, &sigmask, NULL) < 0) {
        fprintf (stderr, "Couldn't block signals: '%s'\n", strerror (errno));
        signal_fd = -1;
    }

    /* Get new FD to read signals from it */
    else if ((signal_fd = signalfd (-1, &sigmask, 0)) < 0) {
        fprintf (stderr, "Couldn't setup signal FD: '%s'\n", strerror (errno));
        signal_fd = -1;
    }   
    return signal_fd;
}

/**
 * @brief Execute the watcher update command.
 * 
 * @param command A command to execute.
 * @param verbose If true report each action.
 */
static int execute_command(const char *command, const bool verbose) {

    if (verbose) {
        printf("Notify event - executing '%s'\n", command);
    }
    return (WIFEXITED(system(command)) == 0) ? 0 : -1;
}

/**
 * @brief Watch a file, files or a directory for changes.
 * @details The routine wathes a file, files or directory for changes
 * and executes a command for each change.
 * 
 * @param watch_target A pointer to the file pattern to scan for changes.
 * @param command A pointer to the command to execute.
 * @param continuous If true the program scans continuously, otherwise scans once.
 * @param verbose Report events and debug information.
 * @return int 0 on success.
 */
int watch_for_changes(const char *watch_target, const char *command, bool continuous, bool verbose) {
    int ret = EXIT_FAILURE;
    int signal_fd;
    int inotify_fd;

    // Initialise the signals interface.
    signal_fd = initialize_signals();
    if (signal_fd == -1) {
        fprintf(stderr, "Unable to initialise signal handler\n");
        ret = EXIT_FAILURE;
    }
    else if ((inotify_fd = initialise_watcher(watch_target, verbose)) == -1) {
        close(signal_fd);
        fprintf(stderr, "Unable to initialise watch handler\n");
        ret = EXIT_FAILURE;
    }
    else {
        // Assume this is going to work (and use this as an error flag).
        ret = EXIT_SUCCESS;

        // Setup a structure of the event handles that we'll watch.
        struct pollfd poll_handles[] = {
            { .fd = signal_fd, .events = POLLIN },
            { .fd = inotify_fd, .events = POLLIN }
        };

        // Now loop through the handles.
        int watch_events = 0;
        for (;;) {
            // Set the timeout to a low value if there are watch events, otherwise 1s.
            int timeout = watch_events ? 100 : 1000;
            
            // Wait for a second.
            int npoll = poll(poll_handles, FD_POLL_MAX, timeout);
            if (npoll == 0) {
                if (watch_events > 0) {
                    watch_events = 0;
                    if (verbose) {
                        printf("Notify event - executing '%s'\n", command);
                    }
                    int rc = system(command);
                    if (verbose) {
                        fprintf(stdout, "return code %x\n", rc);
                    }
                    if (rc != 0) {
                        if (WIFEXITED(rc) != 0) {
                            break;
                        }
                    }
                }
            }
            else if (npoll < 0) {
                fprintf(stderr, "Couldn't poll: '%s'\n", strerror(errno));
                ret = EXIT_FAILURE;
                break;
            }
            else if (npoll > 0) {
                // Check for an interrupt signal.
                if (poll_handles[FD_POLL_SIGNAL].revents & POLLIN) {
                    struct signalfd_siginfo fdsi;
                    if (read (poll_handles[FD_POLL_SIGNAL].fd, &fdsi, sizeof (fdsi)) != sizeof (fdsi)) {
                        fprintf (stderr, "Couldn't read signal, wrong size read\n");
                        ret = EXIT_FAILURE;
                        break;
                    }
                    /* Break loop if we got the expected signal */
                    if (fdsi.ssi_signo == SIGINT || fdsi.ssi_signo == SIGTERM) {
                        if (verbose) {
                            fprintf(stdout, "Received shutdown signal!\n");
                        }
                        break;
                    }
                    else if (verbose) {
                        fprintf (stderr, "Received unexpected signal\n");            
                    }
                }

                // Now check for an inotify (file/directory) event.
                if (poll_handles[FD_POLL_INOTIFY].revents & POLLIN) {
                    if (watch_handler(poll_handles[FD_POLL_INOTIFY].fd, verbose) > 0) {
                        watch_events++;
                    }
                }
            }
        }
        if (verbose) {
            puts("Closing down.");
        }
        shutdown_watcher();
        close(signal_fd);
    }
    return ret;
}

/* End. */
