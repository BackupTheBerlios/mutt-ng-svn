/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "alias.h"

#include "lib/str.h"
#include "lib/list.h"
#include "lib/rx.h"

WHERE void (*mutt_error) (const char *, ...);
WHERE void (*mutt_message) (const char *, ...);

WHERE CONTEXT *Context;

WHERE char Errorbuf[STRING];
WHERE char AttachmentMarker[STRING];

WHERE char Quotebuf[SHORT_STRING];

#if defined(DL_STANDALONE) && defined(USE_DOTLOCK)
WHERE char *MuttDotlock;
#endif

WHERE ADDRESS *EnvFrom;
WHERE ADDRESS *From;

WHERE char *AliasFile;
WHERE char *AliasFmt;
WHERE char *AssumedCharset;
WHERE char *AttachSep;
WHERE char *Attribution;
WHERE char *AttachFormat;
WHERE char *Charset;
WHERE char *ComposeFormat;
WHERE char *ConfigCharset;
WHERE char *ContentType;
WHERE char *DefaultHook;
WHERE char *DateFmt;
WHERE char *DisplayFilter;
WHERE char *DsnNotify;
WHERE char *DsnReturn;
WHERE char *Editor;
WHERE char *EditorHeaders;
WHERE char *EscChar;
WHERE char *FileCharset;
WHERE char *FolderFormat;
WHERE char *ForwFmt;
WHERE char *Fqdn;
WHERE char *HdrFmt;
WHERE char *Homedir;
WHERE char *Hostname;

#ifdef USE_IMAP
WHERE char *ImapAuthenticators INITVAL (NULL);
WHERE char *ImapDelimChars INITVAL (NULL);
WHERE char *ImapHeaders;
WHERE char *ImapHomeNamespace INITVAL (NULL);
WHERE char *ImapLogin INITVAL (NULL);
WHERE char *ImapPass INITVAL (NULL);
WHERE char *ImapUser INITVAL (NULL);
#endif
WHERE char *Inbox;
WHERE char *Ispell;
WHERE char *Locale;
WHERE char *MailcapPath;
WHERE char *Maildir;

#if USE_HCACHE
WHERE char *HeaderCache;
#if HAVE_GDBM || HAVE_DB4
WHERE char *HeaderCachePageSize;
#endif /* HAVE_GDBM || HAVE_DB4 */
#endif /* USE_HCACHE */
WHERE char *MhFlagged;
WHERE char *MhReplied;
WHERE char *MhUnseen;
WHERE char *MsgFmt;
WHERE char *MsgIdFormat;

WHERE rx_t AttachRemindRegexp;
WHERE rx_t Mask;
WHERE rx_t QuoteRegexp;
WHERE rx_t ReplyRegexp;
WHERE rx_t Smileys;
WHERE rx_t GecosMask;
WHERE rx_t StripWasRegexp;

#ifdef USE_SOCKET
WHERE char *Preconnect INITVAL (NULL);
WHERE char *Tunnel INITVAL (NULL);
WHERE short NetInc;
#endif /* USE_SOCKET */

#ifdef MIXMASTER
WHERE char *Mixmaster;
WHERE char *MixEntryFormat;
#endif

WHERE char *OperatingSystem INITVAL (NULL);

WHERE char *Muttrc INITVAL (NULL);

#ifdef USE_NNTP
WHERE char *NewsCacheDir;
WHERE char *GroupFormat;
WHERE char *Inews;
WHERE char *NewsServer;
WHERE char *NntpUser;
WHERE char *NntpPass;
WHERE char *NewsRc;
#endif
WHERE char *Outbox;
WHERE char *Pager;
WHERE char *PagerFmt;
WHERE char *PipeSep;

#ifdef USE_POP
WHERE char *PopAuthenticators INITVAL (NULL);
WHERE short PopCheckTimeout;
WHERE char *PopHost;
WHERE char *PopPass INITVAL (NULL);
WHERE char *PopUser INITVAL (NULL);
#endif
WHERE char *PostIndentString;
WHERE char *Postponed;
WHERE char *Prefix;
WHERE char *PrintCmd;
WHERE char *QueryCmd;
WHERE char *Realname;
WHERE char *SendCharset;
WHERE char *Sendmail;
WHERE char *Shell;
WHERE char *SidebarDelim;
WHERE char *SidebarNumberFormat;
WHERE char *SidebarBoundary;
WHERE char *Signature;
WHERE char *SignOffString;
WHERE char *SimpleSearch;

#if defined(USE_LIBESMTP)
WHERE char *SmtpAuthUser;
WHERE char *SmtpAuthPass;
WHERE char *SmtpHost;
WHERE unsigned short SmtpPort;
WHERE char *SmtpUseTLS;
#endif
WHERE char *Spoolfile;
WHERE char *SpamSep;

#if defined(USE_SSL) || defined(USE_GNUTLS)
WHERE char *SslCertFile INITVAL (NULL);
#endif
#if defined(USE_SSL)
WHERE char *SslEntropyFile INITVAL (NULL);
#endif
#ifdef USE_SSL
WHERE char *SslClientCert INITVAL (NULL);
WHERE LIST *SslSessionCerts INITVAL (NULL);
#endif
#ifdef USE_GNUTLS
WHERE short SslDHPrimeBits;
WHERE char *SslCACertFile INITVAL (NULL);
#endif
WHERE char *StChars;
WHERE char *Status;
WHERE char *Tempdir;
WHERE char *Tochars;
WHERE char *TrashPath;
WHERE char *Username;
WHERE char *Visual;
WHERE char *XtermTitle;
WHERE char *XtermLeave;
WHERE char *XtermIcon;

WHERE char *CurrentFolder;
WHERE char *LastFolder;

WHERE LIST *AutoViewList INITVAL (0);
WHERE LIST *AlternativeOrderList INITVAL (0);
WHERE LIST *AttachAllow INITVAL(0);
WHERE LIST *AttachExclude INITVAL(0);
WHERE LIST *InlineAllow INITVAL(0);
WHERE LIST *InlineExclude INITVAL(0);
WHERE LIST *HeaderOrderList INITVAL (0);
WHERE LIST *Ignore INITVAL (0);
WHERE LIST *MimeLookupList INITVAL (0);
WHERE LIST *UnIgnore INITVAL (0);

WHERE list2_t *Alternates INITVAL (0);
WHERE list2_t *UnAlternates INITVAL (0);
WHERE list2_t *MailLists INITVAL (0);
WHERE list2_t *UnMailLists INITVAL (0);
WHERE list2_t *SubscribedLists INITVAL (0);
WHERE list2_t *UnSubscribedLists INITVAL (0);
WHERE SPAM_LIST *SpamList INITVAL (0);
WHERE list2_t *NoSpamList INITVAL (0);

/* bit vector for boolean variables */
#ifdef MAIN_C
unsigned char Options[(OPTMAX + 7) / 8];
#else
extern unsigned char Options[];
#endif

/* bit vector for the yes/no/ask variable type */
#ifdef MAIN_C
unsigned char QuadOptions[(OPT_MAX * 2 + 7) / 8];
#else
extern unsigned char QuadOptions[];
#endif

WHERE unsigned short Counter INITVAL (0);

#ifdef USE_NNTP
WHERE short NewsPollTimeout;
WHERE short NntpContext;
#endif

WHERE short ConnectTimeout;
WHERE short HistSize;
WHERE short MenuContext;
WHERE short PagerContext;
WHERE short PagerIndexLines;
WHERE short ReadInc;
WHERE short SendmailWait;
WHERE short SleepTime INITVAL (1);
WHERE short Timeout;
WHERE short Umask INITVAL (0077);
WHERE short WrapMargin;
WHERE short WriteInc;
WHERE short MaxDispRecips;
WHERE short MaxLineLength;

WHERE short ScoreThresholdDelete;
WHERE short ScoreThresholdRead;
WHERE short ScoreThresholdFlag;

WHERE short DrawFullLine INITVAL (0);
WHERE short SidebarWidth;

#ifdef USE_IMAP
WHERE short ImapKeepalive;
WHERE short ImapBuffyTimeout;
#endif

/* flags for received signals */
WHERE SIG_ATOMIC_VOLATILE_T SigAlrm INITVAL (0);
WHERE SIG_ATOMIC_VOLATILE_T SigInt INITVAL (0);
WHERE SIG_ATOMIC_VOLATILE_T SigWinch INITVAL (0);

WHERE int CurrentMenu;

WHERE ALIAS *Aliases INITVAL (0);
WHERE LIST *UserHeader INITVAL (0);

/*-- formerly in pgp.h --*/
WHERE rx_t PgpGoodSign;
WHERE char *PgpSignAs;
WHERE short PgpTimeout;
WHERE char *PgpEntryFormat;
WHERE char *PgpClearSignCommand;
WHERE char *PgpDecodeCommand;
WHERE char *PgpVerifyCommand;
WHERE char *PgpDecryptCommand;
WHERE char *PgpSignCommand;
WHERE char *PgpEncryptSignCommand;
WHERE char *PgpEncryptOnlyCommand;
WHERE char *PgpImportCommand;
WHERE char *PgpExportCommand;
WHERE char *PgpVerifyKeyCommand;
WHERE char *PgpListSecringCommand;
WHERE char *PgpListPubringCommand;
WHERE char *PgpGetkeysCommand;

/*-- formerly in smime.h --*/
WHERE char *SmimeDefaultKey;
WHERE char *SmimeCryptAlg;
WHERE short SmimeTimeout;
WHERE char *SmimeCertificates;
WHERE char *SmimeKeys;
WHERE char *SmimeCryptAlg;
WHERE char *SmimeCALocation;
WHERE char *SmimeVerifyCommand;
WHERE char *SmimeVerifyOpaqueCommand;
WHERE char *SmimeDecryptCommand;
WHERE char *SmimeSignCommand;
WHERE char *SmimeSignOpaqueCommand;
WHERE char *SmimeEncryptCommand;
WHERE char *SmimeGetSignerCertCommand;
WHERE char *SmimePk7outCommand;
WHERE char *SmimeGetCertCommand;
WHERE char *SmimeImportCertCommand;
WHERE char *SmimeGetCertEmailCommand;

#ifdef MAIN_C
const char *Weekdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *Months[] =
  { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
"Nov", "Dec", "ERR" };

const char *BodyTypes[] =
  { "x-unknown", "audio", "application", "image", "message", "model",
"multipart", "text", "video" };
const char *BodyEncodings[] =
  { "x-unknown", "7bit", "8bit", "quoted-printable", "base64", "binary",
"x-uuencoded" };
#else
extern const char *Weekdays[];
extern const char *Months[];
#endif

#ifdef MAIN_C
/* so that global vars get included
 * FIXME WTF IS THIS?!?!???!?!??!?!??
 */
#include "mx.h"
#include "buffy.h"
#include "sort.h"
#include "mutt_crypt.h"
#endif /* MAIN_C */

#endif /* !_GLOBALS_H */
