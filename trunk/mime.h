/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* Content-Type */
enum {
  TYPEOTHER,
  TYPEAUDIO,
  TYPEAPPLICATION,
  TYPEIMAGE,
  TYPEMESSAGE,
  TYPEMODEL,
  TYPEMULTIPART,
  TYPETEXT,
  TYPEVIDEO
};

/* Content-Transfer-Encoding */
enum {
  ENCOTHER,
  ENC7BIT,
  ENC8BIT,
  ENCQUOTEDPRINTABLE,
  ENCBASE64,
  ENCBINARY,
  ENCUUENCODED
};

/* Content-Disposition values */
enum {
  DISPINLINE,
  DISPATTACH,
  DISPFORMDATA
};

/* MIME encoding/decoding global vars */

#ifndef _SENDLIB_C
extern int Index_hex[];
extern int Index_64[];
extern char B64Chars[];
#endif

#define hexval(c) Index_hex[(unsigned int)(c)]
#define base64val(c) Index_64[(unsigned int)(c)]

#define is_multipart(x) \
    ((x)->type == TYPEMULTIPART \
     || ((x)->type == TYPEMESSAGE && (!strcasecmp((x)->subtype, "rfc822") \
				      || !strcasecmp((x)->subtype, "news"))))

extern const char *BodyTypes[];
extern const char *BodyEncodings[];

#define TYPE(X) ((X->type == TYPEOTHER) && (X->xtype != NULL) ? X->xtype : BodyTypes[(X->type)])
#define ENCODING(X) BodyEncodings[(X)]

/* other MIME-related global variables */
#ifndef _SENDLIB_C
extern char MimeSpecials[];
#endif
