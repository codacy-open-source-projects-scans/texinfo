#! /bin/sh
# Copyright 2011-2024 Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Originally written by Patrice Dumas.
#
# This script is used by tp/Makefile.am to regenerate Makefile.docstr

dir=`echo $0 | sed 's,/[^/]*$,,'`
outfile=$1

(
cd "$dir/.." || exit 1

if test -f ../po_document/LINGUAS; then :; else
  echo "$0: no ../po_document/LINGUAS, goodbye" 1>&2
  exit 1
fi

basefile=`basename $outfile`
cat >$outfile <<END_HEADER
# $basefile generated by $0.
#
# Copyright Free Software Foundation, Inc.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

END_HEADER

for lingua in `cat ../po_document/LINGUAS`; do
  # Only run by maintainers, so assume GNU make (by using $@ in regular rules).
  # Just for convenience.
  echo '
$(srcdir)/../po_document/'"$lingua.gmo"': $(srcdir)/../po_document/'"$lingua.po"'
	cd ../po_document/ && $(MAKE) $(AM_MAKEFLAGS) '"$lingua.gmo"'

LocaleData/'"$lingua"'/LC_MESSAGES/$(document_domain).mo: $(srcdir)/../po_document/'"$lingua"'.gmo
	$(MKDIR_P) `dirname $@`
	$(INSTALL_DATA) $(srcdir)/../po_document/'"$lingua"'.gmo $@

' >>$outfile

  dependencies="$dependencies "'LocaleData/'"$lingua"'/LC_MESSAGES/$(document_domain).mo'
done

echo 'document_strings_mo_files = '"$dependencies" >>$outfile
)

