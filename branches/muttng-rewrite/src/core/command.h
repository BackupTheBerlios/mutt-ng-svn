/**
 * @file core/command.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @brief Interface: Commands
 */
#ifndef CORE_COMMAND_H
#define CORE_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#define M_DETACH_PROCESS        1

#include <stdio.h>
#include <sys/types.h>

/**
 * Run a command via shell.
 * Basically this is does the same as system() but with
 * more sane signal handling.
 * @param command Command to run.
 * @param flags See M_*.
 * @return Exit status of the command or -1 in case of error.
 */
int command_run (const char *cmd, int flags);

/**
 * Invokes a commmand on a pipe and optionally connects its stdin and stdout
 * to the specified handles.
 * @param cmd Command to run.
 * @param in Optional handle for input.
 * @param out Optional handle for normal output.
 * @param err Optional handle for error output.
 * @param fdin Input file descriptor.
 * @param fdout Normal output file descriptor.
 * @param fderr Error output file descriptor.
 * @return pid of filter process.
 */
pid_t command_filter_fd(const char *cmd, FILE ** in, FILE ** out, FILE ** err,
                        int fdin, int fdout, int fderr);

#define command_filter(CMD,IN,OUT,ERR) command_filter_fd(CMD,IN,OUT,ERR,-1,-1,-1)

/**
 * Wait for a previously created filter process to finish.
 * @param pid Process ID as returned by command_filter_fd() or
 * command_filter().
 * @return Exit status or -1 in case of error.
 */
int command_filter_wait (pid_t pid);

#ifdef __cplusplus
}
#endif

#endif /* !CORE_COMMAND_H */
