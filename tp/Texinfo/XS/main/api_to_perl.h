/* tree_perl_api.h - API to interact with perl from C code */
#ifndef TREE_PERL_API_H
#define TREE_PERL_API_H

#include "tree_types.h"
#include "converter_types.h"

void unregister_perl_tree_element (ELEMENT *e);
void unregister_perl_button (BUTTON_SPECIFICATION *button);
void register_perl_button (BUTTON_SPECIFICATION *button);
void unregister_document_hv (DOCUMENT *document);
/* HTML specific */
void unregister_html_converter_perl_hv (CONVERTER *converter);

void call_switch_to_global_locale (void);
void call_sync_locale (void);
void croak_message (char *message);

char *get_perl_scalar_reference_value (const void *sv_string);
#endif
