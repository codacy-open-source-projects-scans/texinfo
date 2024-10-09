#!/usr/bin/perl

use strict;

my $input_file = $ARGV[0];

if (!$input_file) {
    warn "usage: adjust_translations.pl LL.po\n";
    exit 1;
}

my $fh;
if (!open($fh, '<', $input_file)) {
    warn "error opening $input_file for reading\n";
    exit 1;
}

# Read entire contents of file.
$/ = undef;
my $data = <$fh>;
if (!close ($fh)) {
    warn "error closing $input_file\n";
    exit 1;
}

# Strip out lines marking translations as fuzzy if a message context is
# provided.  This is because translations contexts were added to several
# strings, so translations will not work unless or until translation
# files are updated at the Translation Project.
#
$data =~ s/#, fuzzy\n((#.*\n)*msgctxt)/$1/g;

# Translated string changed -- to ---
$data =~ s/#, fuzzy, perl-brace-format\n#\| msgid "\@tie\{\}-- .*\n(msgid "\@tie\{)/$1/g;
$data =~ s/msgstr "\@tie\{\}-- /msgstr "\@tie\{\}--- /g;
$data =~ s/msgstr "\@tie\{ \}-- /msgstr "\@tie\{\}--- /g;

# Overwrite original file
print $data >$input_file;
if (!open($fh, '>', $input_file)) {
    warn "error opening $input_file for writing\n";
    exit 1;
}

print $fh $data;

if (!close ($fh)) {
    warn "error closing $input_file\n";
    exit 1;
}

exit 0;


