/**
 * @file core/command.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @brief Interface: Commands
 */
#ifndef CORE_COMMAND_H
#define CORE_COMMAND_H

#define M_DETACH_PROCESS        1

/**
 * Run a command via shell.
 * Basically this is does the same as system() but with
 * more sane signal handling.
 * @param command Command to run.
 * @param flags See M_*.
 * @return Exit status of the command or -1 in case of error.
 */
int command_run (const char *cmd, int flags);

#endif
