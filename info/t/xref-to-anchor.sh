#!/bin/sh
# Copyright (C) 2015-2024 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

srcdir=${srcdir:-.}
. $srcdir/t/Init-test.inc

run_ginfo -f xref_to_anchor
# Follow cross-reference to an anchor
printf '\t\r' >&7

# Follow the cross-reference on the line after the anchor
printf '\6\6\rDq' >&7

timeout_test

if test ! -f $ginfo_output; then
  retval=1
else
  # Return non-zero (test failure) if files differ
  diff $ginfo_output $t/node-target
  retval=$?
fi

cleanup
