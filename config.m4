dnl $Id: config.m4 213 2007-03-11 20:37:47Z j $
dnl config.m4 for extension dav

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(tnc_dav, for dav support,
[  --with-tnc-dav             Include TNC dav support])

PHP_ARG_WITH(neon_config, path to libneon install,
[  --with-neon-config=PATH       Path to neon-config executable], no, no)

if test "$PHP_TNC_DAV" != "no"; then
  AC_MSG_CHECKING([for neon-config])
  if test "$PHP_NEON_CONFIG" != "no"; then
    if test "$PHP_NEON_CONFIG" == "yes"; then
      AC_MSG_ERROR([Must specify path when using --with-neon-config])
    fi
    NEON_CONFIG=$PHP_NEON_CONFIG
  else
    NEON_CONFIG=`$php_shtool path neon-config`
  fi


  dnl Check for libneon install
  if test -f "$NEON_CONFIG"; then
    DAV_LIBS=$($NEON_CONFIG --libs)
    DAV_INCS=$($NEON_CONFIG --cflags)


    PHP_EVAL_LIBLINE($DAV_LIBS, TNC_DAV_SHARED_LIBADD)
    PHP_EVAL_INCLINE($DAV_INCS)
    
    PHP_SUBST(TNC_DAV_SHARED_LIBADD)
    PHP_NEW_EXTENSION(tnc_dav, tnc_dav.c, $ext_shared)  
    
  else
    AC_MSG_ERROR([Could not find neon-config])
  fi
else
  AC_MSG_ERROR([Please specify --with-tnc_dav to enable the TncWebdav extension])
fi
