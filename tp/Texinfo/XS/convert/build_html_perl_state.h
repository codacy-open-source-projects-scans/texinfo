/* build_html_perl_state.h - declarations for build_html_perl_state.c */
#ifndef BUILD_HTML_PERL_STATE_H
#define BUILD_HTML_PERL_STATE_H

#include "EXTERN.h"
#include "perl.h"

#include "tree_types.h"
#include "converter_types.h"
/* for NAMED_STRING_ELEMENT_LIST */
#include "translations.h"

HV *build_html_target (const HTML_TARGET *html_target);
void build_html_translated_names (HV *hv, CONVERTER *converter);

void html_pass_conversion_initialization (CONVERTER *converter,
                                     SV *converter_sv, SV *document_in);
void html_pass_converter_initialization_state (const CONVERTER *converter,
                                          HV *converter_hv, SV *document_in);

void html_pass_converter_setup_state (const CONVERTER *converter,
                                      SV *converter_sv);

void html_pass_conversion_output_units (CONVERTER *converter, SV *converter_sv,
                                   SV **output_units_sv, SV **special_units_sv,
                                   SV **associated_special_units_sv);

void html_pass_units_directions_files (CONVERTER *converter,
                                  SV *converter_sv, SV *output_units_sv,
                                  SV *special_units_sv,
                                  SV *associated_special_units_sv);

void html_pass_output_units_global_targets (CONVERTER *self,
                                            SV *output_units_sv,
                        SV *special_units_sv, SV *associated_special_units_sv);

SV *build_html_files_source_info
                          (const FILE_SOURCE_INFO_LIST *files_source_info);
void pass_html_global_units_directions (SV *converter_sv,
                       const OUTPUT_UNIT **global_units_directions,
                   const SPECIAL_UNIT_DIRECTION *special_units_direction_name);
void pass_html_elements_in_file_count (SV *converter_sv,
                   FILE_NAME_PATH_COUNTER_LIST *output_unit_files);

void build_html_formatting_state (CONVERTER *converter);

SV *build_html_command_formatted_args
           (const HTML_ARGS_FORMATTED *args_formatted);
SV *build_replaced_substrings (NAMED_STRING_ELEMENT_LIST *replaced_substrings);

void build_pending_footnotes (AV *av, HTML_PENDING_FOOTNOTE_STACK *stack);

void build_simpletitle (const CONVERTER *converter, HV *converter_info_hv);
void pass_jslicenses (const JSLICENSE_CATEGORY_LIST *jslicenses,
                      HV *converter_info_hv);

SV *pass_sv_converter_info (const CONVERTER *converter,
                            const char *converter_info, SV *converter_sv);
#endif
