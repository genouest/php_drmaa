dnl $Id$
dnl config.m4 for extension drmaa

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(drmaa, for drmaa support,
dnl Make sure that the comment is aligned:
[  --with-drmaa[=DIR]             Include drmaa support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(drmaa, whether to enable drmaa support,
dnl Make sure that the comment is aligned:
dnl [  --enable-drmaa           Enable drmaa support])

if test "$PHP_DRMAA" != "no"; then
  dnl Write more examples of tests here...

  # --with-drmaa -> check with-path
  #/usr/local/drmaa/lib/lx24-amd64
  SEARCH_PATH="/usr/local /usr /usr/local/drmaa"     # you might want to change this
  SEARCH_FOR="/lib/libdrmaa.so"  # you most likely want to change this
  if test -r $PHP_DRMAA/$SEARCH_FOR; then # path given as parameter
    DRMAA_DIR=$PHP_DRMAA
  else # search default path list
    AC_MSG_CHECKING([for drmaa files in default path])
     for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
       DRMAA_DIR=$i
       AC_MSG_RESULT(found in $i)
      fi
     done
  fi

  if test -z "$DRMAA_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the drmaa distribution])
  fi

  # --with-drmaa -> add include path
  PHP_ADD_INCLUDE($DRMAA_DIR/include)

  # --with-drmaa -> check for lib and symbol presence
  LIBNAME=drmaa # you may want to change this
  dnl LIBSYMBOL=drmaa # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DRMAA_DIR/lib, DRMAA_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_DRMAALIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong drmaa lib version or lib not found])
  dnl ],[
  dnl   -L$DRMAA_DIR/lib -lm -ldl
  dnl ])
  dnl
  PHP_SUBST(DRMAA_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DRMAA_DIR/lib, DRMAA_SHARED_LIBADD)
  AC_DEFINE(HAVE_DRMAALIB,1,[ ])

  dnl PHP_ADD_INCLUDE("/usr/local/drmaa/include")

  PHP_NEW_EXTENSION(drmaa, drmaa.c, $ext_shared)
fi
