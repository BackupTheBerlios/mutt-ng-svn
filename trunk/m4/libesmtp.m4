dnl vim:ft=config:

dnl Search for libesmtp, by Steven Engelhardt <sengelha@yahoo.com>
dnl
dnl libesmtp often requires linking against -lpthread or -lc_r (BSD).
dnl This macro attempts to centralize this code.

AC_DEFUN([MUTT_AM_LIBESMTP],
[
  AC_ARG_WITH(
    [libesmtp],
    [  --with-libesmtp=DIR      Compile in support for libesmtp for the MTA],
    [ if test "$with_libesmtp" != "no"
      then
        mutt_libesmtp_check_path="$PATH"

        if test "$with_libesmtp" != "yes"
        then
          mutt_libesmtp_check_path="$tmp_path:$withval/bin"
        fi

        dnl 1. Find libesmtp-config
        AC_PATH_PROG([mutt_libesmtp_config_path],
                     [libesmtp-config],
                     [no],
                     [$mutt_libesmtp_check_path])

        if test "$mutt_libesmtp_config_path" = "no"
        then
          AC_MSG_ERROR([libesmtp-config binary not found.])
        fi

        dnl 2. Get CFLAGS and LIBS from libesmtp-config
        mutt_libesmtp_cflags=`$mutt_libesmtp_config_path --cflags`
        mutt_libesmtp_libs=`$mutt_libesmtp_config_path --libs`

        dnl 3. Verify libesmtp.h can be found with these settings
        temp_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $mutt_libesmtp_cflags"
        AC_CHECK_HEADER([libesmtp.h],
                        [],
                        AC_MSG_ERROR([Could not find libesmtp.h]))
        CFLAGS="$temp_CFLAGS"

        dnl 4. Verify the libesmtp library can be linked in
        temp_CFLAGS="$CFLAGS"
        temp_LIBS="$LIBS"
        CFLAGS="$CFLAGS $mutt_libesmtp_cflags"
        LIBS="$LIBS $mutt_libesmtp_libs"
        AC_CHECK_LIB([esmtp],
                     [smtp_create_session],
                     [],
                     AC_MSG_ERROR([Could not find libesmtp]))
        CFLAGS="$temp_CFLAGS"
        LIBS="$temp_LIBS"

        dnl 5. Export use_libesmtp variable so configure.in can
        dnl    act accordingly.
        use_libesmtp=yes
      fi
    ]
  )
])
