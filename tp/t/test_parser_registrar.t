use strict;

use lib '.';
use Texinfo::ModulePath (undef, undef, undef, 'updirs' => 2);

use Test::More;

BEGIN { plan tests => 9; }

use Texinfo::Parser;
use Texinfo::Report;

# For consistent error messages, use the C locale
$ENV{LC_ALL} = 'C';
$ENV{LANGUAGE} = 'en';


ok(1, "modules loading");

my $registrar = Texinfo::Report::new();

my $initial_error = "Initial error";
$registrar->document_error($initial_error);

my $parser = Texinfo::Parser::parser({'registrar' => $registrar});

# this also tests errors with line_errors not defined
my $document = $parser->parse_texi_piece("\@end format\n");

my $parser_registrar = $parser->registrar();
ok ($parser_registrar eq $registrar, 'reused registrar');

my ($error_warnings_list, $error_count) = $parser_registrar->errors();

is ($error_count, 2, 'error count');

# line_nr is undef with document_error()
my @errors_references = ([undef, $initial_error],
                         [1, 'unmatched `@end format\'']);

my $error_idx = 0;
foreach my $error_message (@$error_warnings_list) {
  my ($error_line_nr_reference, $error_line_reference)
          = @{$errors_references[$error_idx]};
  is ($error_message->{'type'}, 'error', "error type $error_idx");
  ok (((not defined($error_message->{'line_nr'})
       and not defined($error_line_nr_reference))
      or $error_message->{'line_nr'} == $error_line_nr_reference),
      "error line $error_idx");
  is ($error_message->{'error_line'}, $error_line_reference."\n",
      "error message $error_idx");
  $error_idx++;
}
