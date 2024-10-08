#include "main/text.h"

int xspara_new (HV *conf);
void xspara_init_state (HV *hash);
void xspara_set_state (SV *state);
TEXT xspara_add_next (char *, int, int transparent);
TEXT xspara_add_text (char *, int);
void xspara_set_space_protection (int space_protection, int ignore_columns,
       int keep_end_lines, int french_spacing, int double_width_no_break);
void xspara__end_line (void);
char *xspara_end_line (void);
char *xspara_get_pending (void);
char *xspara_end (void);
char *xspara_add_pending_word (int add_spaces);
void xspara_allow_end_sentence (void);
void xspara_remove_end_sentence (void);
void xspara_add_end_sentence (int value);
int xspara_end_line_count (void);
int xspara_counter (void);
