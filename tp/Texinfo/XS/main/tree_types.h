/* tree_types.h - types for the parse tree that are used in many places */
#ifndef TREE_TYPES_H
#define TREE_TYPES_H
/* Copyright 2010-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stddef.h>
#include <stdint.h>

#include "command_ids.h"
#include "element_types.h"
#include "text.h"

enum extra_type {
   extra_element,
   extra_element_oot,
   extra_contents,
   extra_directions,
   extra_text,
   extra_misc_args,
   extra_string,
   extra_integer,
   extra_deleted
};

/* the *_none enums are not necessarily used, they may also
   be there to avoid using 0, for a code easier to debug */

#define SM_TYPES_LIST \
   sm_type(none) \
   sm_type(include) \
   sm_type(setfilename) \
   sm_type(delcomment) \
   sm_type(defline_continuation) \
   sm_type(macro_expansion) \
   sm_type(linemacro_expansion) \
   sm_type(value_expansion) \
   sm_type(ignored_conditional_block) \
   sm_type(expanded_conditional_command) \

enum source_mark_type {
  #define sm_type(name) SM_type_ ## name,
   SM_TYPES_LIST
  #undef sm_type
};

enum source_mark_status {
   SM_status_none,
   SM_status_start,
   SM_status_end,
};

/* need to be in the same order as explicit nodes directions */
enum directions {
   D_next,
   D_prev,
   D_up,
};

enum output_unit_type {
   OU_unit,
   OU_external_node_unit,
   OU_special_unit,
};

/* relative output unit directions */
#define RUD_DIRECTIONS_TYPES_LIST \
   rud_type(This) \
   rud_type(Forward) \
   rud_type(Back) \
   rud_type(FastForward) \
   rud_type(FastBack) \
   rud_type(Next) \
   rud_type(Prev) \
   rud_type(Up) \
   rud_type(SectionNext) \
   rud_type(SectionPrev) \
   rud_type(SectionUp) \
   rud_type(NodeNext) \
   rud_type(NodeForward) \
   rud_type(NodeBack) \
   rud_type(NodePrev) \
   rud_type(NodeUp)

/* relative output unit file directions */
#define RUD_FILE_DIRECTIONS_TYPES \
   rud_type(PrevFile) \
   rud_type(NextFile)

enum relative_unit_direction_type {
  #define rud_type(name) RUD_type_## name,
   RUD_DIRECTIONS_TYPES_LIST
   RUD_FILE_DIRECTIONS_TYPES
  #undef rud_type
  #define rud_type(name) RUD_type_FirstInFile## name,
   RUD_DIRECTIONS_TYPES_LIST
  #undef rud_type
};

/* in output_units.c */
extern char *relative_unit_direction_name[];

typedef struct KEY_PAIR {
    char *key;
    enum extra_type type;
    intptr_t value;
} KEY_PAIR;

typedef struct ELEMENT_LIST {
    struct ELEMENT **list;
    size_t number;
    size_t space;
} ELEMENT_LIST;

typedef struct SOURCE_INFO {
    int line_nr;
    char *file_name;
    char *macro;
} SOURCE_INFO;

typedef struct ASSOCIATED_INFO {
    KEY_PAIR *info;
    size_t info_number;
    size_t info_space;
} ASSOCIATED_INFO;

typedef struct SOURCE_MARK {
    enum source_mark_type type;
    enum source_mark_status status;
    size_t position;
    int counter;
    struct ELEMENT *element; /* needed for elements removed
                                from the tree */
    char *line;  /* used when the information is not available as
                    an element, for DEL comments, for instance */
} SOURCE_MARK;

typedef struct SOURCE_MARK_LIST {
    struct SOURCE_MARK **list;
    size_t number;
    size_t space;
} SOURCE_MARK_LIST;

/* structure used after splitting output units.  Could have been defined
   in another file as it is not related to element trees.  However, it
   is used in ELEMENT, so it is defined here */
typedef struct OUTPUT_UNIT {
    /* Used when building Perl tree only. This should be HV *hv,
       but we don't want to include the Perl headers everywhere; */
    void *hv;

    enum output_unit_type unit_type;
    size_t index;
    struct ELEMENT *unit_command;
    char *unit_filename;
    ELEMENT_LIST unit_contents;
    struct OUTPUT_UNIT *tree_unit_directions[2];

    struct OUTPUT_UNIT *first_in_page;
    struct OUTPUT_UNIT *directions[RUD_type_FirstInFileNodeUp+1];

    /* for special output units only */
    /* could be an enum as for now new special types cannot be customized
       but lets keep it an option */
    char *special_unit_variety;
    /* for special units associated to a document output unit */
    struct OUTPUT_UNIT *associated_document_unit;
} OUTPUT_UNIT;

/* Could be elsewhere, but it is practical to have it here as it is used
   in build_perl_info.c, for example */
typedef struct OUTPUT_UNIT_LIST {
    struct OUTPUT_UNIT **list;
    size_t number;
    size_t space;
} OUTPUT_UNIT_LIST;

typedef struct ELEMENT {
    /* Used when building Perl tree only. This should be HV *hv,
       but we don't want to include the Perl headers everywhere; */
    void *hv;

    enum element_type type;
    enum command_id cmd;
    TEXT text;
    ELEMENT_LIST args;
    ELEMENT_LIST contents;
    struct ELEMENT *parent;
    SOURCE_INFO source_info;

    ASSOCIATED_INFO extra_info;
    ASSOCIATED_INFO info_info;

    SOURCE_MARK_LIST source_mark_list;

    OUTPUT_UNIT *associated_unit;
} ELEMENT;

typedef struct IGNORED_CHARS {
    int backslash;
    int hyphen;
    int lessthan;
    int atsign;
} IGNORED_CHARS;

typedef struct {
    char *index_name; /* kept with the entry as the indices may be merged */
    ELEMENT *entry_element;
    ELEMENT *entry_associated_element; /* set if the entry is reassociated to
                                          another element */
} INDEX_ENTRY;

typedef struct INDEX {
    char *name;
    char *prefix;
    int in_code;

    struct INDEX *merged_in; /* Index this index is merged into, if any. */

    INDEX_ENTRY *index_entries;
    size_t index_number;
    size_t index_space;
} INDEX;

/* not used in parser */
typedef struct INDEX_ENTRY_AND_INDEX {
    INDEX *index;
    INDEX_ENTRY *index_entry;
} INDEX_ENTRY_AND_INDEX;

/* See parse_node_manual function. */
typedef struct {
    ELEMENT *manual_content;
    ELEMENT *node_content;
    ELEMENT **out_of_tree_elements;
} NODE_SPEC_EXTRA;

typedef struct {
    char *arg_type;
    ELEMENT *element;
} DEF_ARG;

typedef struct {
    size_t label_number;
    char *identifier;
    ELEMENT *element;
} LABEL;

typedef struct {
    size_t number;
    size_t space;
    LABEL *list;
} LABEL_LIST;

typedef struct {
    char *type;
    ELEMENT *element;
} FLOAT_RECORD;

typedef struct {
    size_t number;
    size_t space;
    FLOAT_RECORD *float_types;
} FLOAT_RECORD_LIST;

typedef struct STRING_LIST {
    char **list;
    size_t number;
    size_t space;
} STRING_LIST;

#endif