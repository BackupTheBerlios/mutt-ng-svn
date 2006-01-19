/**
 * @file core/command.c
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @brief Implementation: Commands
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sigs.h"
#include "command.h"
#include "core_features.h"

int command_run (const char *cmd, int flags) {
  int rc = -1;
  struct sigaction act;
  struct sigaction oldtstp;
  struct sigaction oldcont;
  sigset_t set;
  pid_t thepid;

  if (!cmd || !*cmd)
    return (0);

  /* must ignore SIGINT and SIGQUIT */

  sigs_block_signals_system ();

  /* also don't want to be stopped right now */
  if (flags & M_DETACH_PROCESS) {
    sigemptyset (&set);
    sigaddset (&set, SIGTSTP);
    sigprocmask (SIG_BLOCK, &set, NULL);
  }
  else {
    act.sa_handler = SIG_DFL;
    /* we want to restart the waitpid() below */
#ifdef SA_RESTART
    act.sa_flags = SA_RESTART;
#endif
    sigemptyset (&act.sa_mask);
    sigaction (SIGTSTP, &act, &oldtstp);
    sigaction (SIGCONT, &act, &oldcont);
  }

  if ((thepid = fork ()) == 0) {
    act.sa_flags = 0;

    if (flags & M_DETACH_PROCESS) {
      int fd;
      (void)fd;

      /* give up controlling terminal */
      setsid ();

      switch (fork ()) {
      case 0:
#if defined(OPEN_MAX)
        for (fd = 0; fd < OPEN_MAX; fd++)
          close (fd);
#elif defined(_POSIX_OPEN_MAX)
        for (fd = 0; fd < _POSIX_OPEN_MAX; fd++)
          close (fd);
#else
        close (0);
        close (1);
        close (2);
#endif
        chdir ("/");
        act.sa_handler = SIG_DFL;
        sigaction (SIGCHLD, &act, NULL);
        break;

      case -1:
        _exit (127);

      default:
        _exit (0);
      }
    }

    /* reset signals for the child; not really needed, but... */
    sigs_unblock_signals_system (0);
    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction (SIGTERM, &act, NULL);
    sigaction (SIGTSTP, &act, NULL);
    sigaction (SIGCONT, &act, NULL);

    execl (CORE_SHELL, "sh", "-c", cmd, NULL);
    _exit (127);                /* execl error */
  }
  else if (thepid != -1) {
#ifndef USE_IMAP
    /* wait for the (first) child process to finish */
    waitpid (thepid, &rc, 0);
#else
    rc = imap_wait_keepalive (thepid);
#endif
  }

  sigaction (SIGCONT, &oldcont, NULL);
  sigaction (SIGTSTP, &oldtstp, NULL);

  /* reset SIGINT, SIGQUIT and SIGCHLD */
  sigs_unblock_signals_system (1);
  if (flags & M_DETACH_PROCESS)
    sigprocmask (SIG_UNBLOCK, &set, NULL);

  rc = (thepid != -1) ? (WIFEXITED (rc) ? WEXITSTATUS (rc) : -1) : -1;

  return (rc);
}

pid_t command_filter_fd (const char *cmd, FILE ** in, FILE ** out, FILE ** err,
                         int fdin, int fdout, int fderr) {
  int pin[2], pout[2], perr[2], thepid;

  if (in) {
    *in = 0;
    if (pipe (pin) == -1)
      return (-1);
  }

  if (out) {
    *out = 0;
    if (pipe (pout) == -1) {
      if (in) {
        close (pin[0]);
        close (pin[1]);
      }
      return (-1);
    }
  }

  if (err) {
    *err = 0;
    if (pipe (perr) == -1) {
      if (in) {
        close (pin[0]);
        close (pin[1]);
      }
      if (out) {
        close (pout[0]);
        close (pout[1]);
      }
      return (-1);
    }
  }

  sigs_block_signals_system ();

  if ((thepid = fork ()) == 0) {
    sigs_unblock_signals_system (0);

    if (in) {
      close (pin[1]);
      dup2 (pin[0], 0);
      close (pin[0]);
    }
    else if (fdin != -1) {
      dup2 (fdin, 0);
      close (fdin);
    }

    if (out) {
      close (pout[0]);
      dup2 (pout[1], 1);
      close (pout[1]);
    }
    else if (fdout != -1) {
      dup2 (fdout, 1);
      close (fdout);
    }

    if (err) {
      close (perr[0]);
      dup2 (perr[1], 2);
      close (perr[1]);
    }
    else if (fderr != -1) {
      dup2 (fderr, 2);
      close (fderr);
    }

    execl (CORE_SHELL, "sh", "-c", cmd, NULL);
    _exit (127);
  }
  else if (thepid == -1) {
    sigs_unblock_signals_system (1);

    if (in) {
      close (pin[0]);
      close (pin[1]);
    }

    if (out) {
      close (pout[0]);
      close (pout[1]);
    }

    if (err) {
      close (perr[0]);
      close (perr[1]);
    }

    return (-1);
  }

  if (out) {
    close (pout[1]);
    *out = fdopen (pout[0], "r");
  }

  if (in) {
    close (pin[0]);
    *in = fdopen (pin[1], "w");
  }

  if (err) {
    close (perr[1]);
    *err = fdopen (perr[0], "r");
  }

  return (thepid);
}

int command_filter_wait (pid_t pid) {
  int rc;
  if (pid < 0) return 0;
  waitpid (pid, &rc, 0);
  sigs_unblock_signals_system (1);
  rc = WIFEXITED (rc) ? WEXITSTATUS (rc) : -1;
  return rc;
}
