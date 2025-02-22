# copy-file.m4
# serial 5
dnl Copyright (C) 2003, 2009-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_COPY_FILE],
[
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CHECK_FUNCS([chown])
])
