/* Copyright 2010-2024 Free Software Foundation, Inc.

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

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iconv.h>
#include "unistr.h"

/* for isolate_last_space and global_documentlanguage */
#include "parser.h"
#include "command_ids.h"
#include "tree_types.h"
#include "text.h"
#include "tree.h"
#include "builtin_commands.h"
#include "extra.h"
/* for whitespace_chars fatal def_aliases */
#include "utils.h"
/* for relocate_source_marks */
#include "manipulate_tree.h"
#include "source_marks.h"
#include "commands.h"
#include "unicode.h"

void
gather_def_item (ELEMENT *current, enum command_id next_command)
{
  enum element_type type;
  ELEMENT *def_item;
  int contents_count, i;

  if (next_command
      && next_command != CM_defline && next_command != CM_deftypeline)
    type = ET_inter_def_item; /* Between @def*x and @def*. */
  else
    type = ET_def_item;

  if (!current->cmd)
    return;

  /* Check this isn't an "x" type command.
     "This may happen for a construct like:
     @deffnx a b @section
     but otherwise the end of line will lead to the command closing." */
  if (command_data(current->cmd).flags & CF_line)
    return;

  contents_count = current->contents.number;
  if (contents_count == 0)
    return;

  /* Starting from the end, collect everything that is not a ET_def_line and
     put it into the ET_def_item. */
  def_item = new_element (type);
  for (i = 0; i < contents_count; i++)
    {
      ELEMENT *last_child, *item_content;
      last_child = last_contents_child (current);
      if (last_child->type == ET_def_line)
        break;
      item_content = pop_element_from_contents (current);
      insert_into_contents (def_item, item_content, 0);
    }

  if (def_item->contents.number > 0)
    {
      if (current->cmd == CM_defblock
        /* all content between @defblock and first @def*line */
          && def_item->contents.number == contents_count)
        {
          def_item->type = ET_before_defline;
        }
      add_to_element_contents (current, def_item);
    }
  else
    destroy_element (def_item);
}


/* Starting at I in the contents, return the next non-whitespace element,
   incrementing I.  Return null if no more elements. */
ELEMENT *
next_bracketed_or_word_agg (ELEMENT *current, int *i)
{
  int num = 0;
  ELEMENT *new;
  ELEMENT *e;
  int j;
  while (1)
    {
      if (*i == current->contents.number)
        break;
      e = current->contents.list[*i];
      if (e->type == ET_spaces
          || e->type == ET_delimiter)
        {
          if (num > 0)
            break;
          else
            {
              (*i)++;
            }
        }
      else
        {
         /* e is a text or command element or bracketed argument */
          (*i)++;
          num++;
        }
    }

  if (num == 0)
    return 0;

  if (num == 1)
    {
      e = current->contents.list[*i - 1];

      /* there is only one bracketed element */
      if (e->type == ET_bracketed_arg
          || e->type == ET_def_line_arg
          || e->type == ET_untranslated_def_line_arg)
        return e;
    }

  new = new_element (ET_def_line_arg);
  for (j = 0; j < num; j++)
    {
      add_to_element_contents (new,
                               remove_from_contents (current, *i - num));
      /* Note: if we did a lot of this could write a special
         "splicing" function. */
    }
  insert_into_contents (current, new, *i - num);
  *i = *i - num + 1;
  return new;
}

typedef struct {
    enum command_id command;
    enum command_id *argument_types;
} DEF_MAP;

  /*
     Meaning of these:
     CATEGORY - type of entity, e.g. "Function"
     CLASS - class for object-oriented programming
     TYPE - data type of a variable or function return value
     NAME - name of entity being documented
     ARGUMENTS - arguments to a function or macro                  */

enum command_id defline_types[] = {ET_def_category, ET_def_name, ET_def_arg, 0};
enum command_id deftypeline_types[] = {ET_def_category, ET_def_type, ET_def_name, ET_def_typearg, 0};
enum command_id defvr_types[] = {ET_def_category, ET_def_name, 0};
enum command_id deftypefn_types[] = {ET_def_category, ET_def_type, ET_def_name, ET_def_typearg, 0};
enum command_id deftypeop_types[] = {ET_def_category, ET_def_class , ET_def_type, ET_def_name, ET_def_typearg, 0};
enum command_id deftypevr_types[] = {ET_def_category, ET_def_type, ET_def_name, 0};
enum command_id defcv_types[] = {ET_def_category, ET_def_class , ET_def_name, 0};
enum command_id deftypecv_types[] = {ET_def_category, ET_def_class , ET_def_type, ET_def_name, 0};
enum command_id defop_types[] = {ET_def_category, ET_def_class , ET_def_name, ET_def_arg, 0};
enum command_id deftp_types[] = {ET_def_category, ET_def_name, ET_def_typearg, 0};

DEF_MAP def_maps[] = {
  CM_defline, defline_types,
  CM_deftypeline, deftypeline_types,
  CM_deffn, defline_types,
  CM_defvr, defvr_types,
  CM_deftypefn, deftypefn_types,
  CM_deftypeop, deftypeop_types,
  CM_deftypevr, deftypevr_types,
  CM_defcv, defcv_types,
  CM_deftypecv, deftypecv_types,
  CM_defop, defop_types,
  CM_deftp, deftp_types,
};

/* Split non-space text elements into strings without [ ] ( ) , and single
   character strings with one of them. */
static void
split_delimiters (ELEMENT *current, int starting_idx)
{
  int i;
  static char *chars = "[](),";
  for (i = starting_idx; i < current->contents.number; i++)
    {
      ELEMENT *e = current->contents.list[i];
      char *p;
      ELEMENT *new;
      int len;
      /* count UTF-8 encoded Unicode characters for source marks locations */
      uint8_t *u8_text = 0;
      size_t current_position;
      uint8_t *u8_p;
      size_t u8_len;

      if (e->type != ET_NONE && (e->type == ET_bracketed_arg
                                 || e->text.end > 0))
        continue;
      else if (e->text.end == 0)
        {
          new = new_element (ET_def_line_arg);
          new->parent = e->parent;
          add_to_element_contents (new, e);
          current->contents.list[i] = new;
          continue;
        }

      p = e->text.text;

      if (e->source_mark_list.number)
        {
          u8_text = utf8_from_string (p);
          u8_p = u8_text;

          current_position = 0;
        }

      while (1)
        {
          if (strchr (chars, *p))
            {
              new = new_element (ET_delimiter);
              text_append_n (&new->text, p, 1);

              if (u8_text)
                {
                  u8_len = u8_mbsnlen (u8_p, 1);
                  u8_p += u8_len;

                  current_position
                   = relocate_source_marks (&(e->source_mark_list), new,
                                                 current_position, u8_len);
                }

              insert_into_contents (current, new, i++);
              if (!*++p)
                break;
              continue;
            }

          ELEMENT *new_text = new_element (ET_NONE);
          len = strcspn (p, chars);
          text_append_n (&new_text->text, p, len);

          if (u8_text)
            {
              u8_len = u8_mbsnlen (u8_p, len);
              u8_p += u8_len;

             current_position
               = relocate_source_marks (&(e->source_mark_list), new_text,
                                          current_position, u8_len);
            }

          new = new_element (ET_def_line_arg);
          add_to_element_contents (new, new_text);

          insert_into_contents (current, new, i++);
          if (!*(p += len))
            break;
        }
      destroy_element (remove_from_contents (current, i--));
      free (u8_text);
    }
}


/* Divide any text elements into separate elements, separating whitespace
   and non-whitespace. */
static void
split_def_args (ELEMENT *current, int starting_idx)
{
  int i;
  for (i = starting_idx; i < current->contents.number; i++)
    {
      ELEMENT *e = current->contents.list[i];
      char *p;
      ELEMENT *new;
      int len;
      /* count UTF-8 encoded Unicode characters for source marks locations */
      uint8_t *u8_text = 0;
      size_t current_position;
      uint8_t *u8_p;
      size_t u8_len;

      if (e->type == ET_bracketed_arg)
        {
          isolate_last_space (e);
          continue;
        }

      if (e->text.end == 0)
        continue;

      if (e->type == ET_spaces)
        {
          int status;
          int inserted = lookup_info_integer (e, "inserted", &status);
          if (inserted > 0)
            continue;
        }

      p = e->text.text;

      if (e->source_mark_list.number)
        {
          u8_text = utf8_from_string (p);
          u8_p = u8_text;

          current_position = 0;
        }

      while (1)
        {
          len = strspn (p, whitespace_chars);
          if (len)
            {
              new = new_element (ET_spaces);
            }
          else
            {
              len = strcspn (p, whitespace_chars);
              new = new_element (ET_NONE);
            }

          text_append_n (&new->text, p, len);

          if (u8_text)
            {
              u8_len = u8_mbsnlen (u8_p, len);
              u8_p += u8_len;

              current_position
                = relocate_source_marks (&(e->source_mark_list), new,
                                         current_position, u8_len);
            }
          insert_into_contents (current, new, i++);
          if (!*(p += len))
            break;
        }
      destroy_element (remove_from_contents (current, i--));
      free (u8_text);
    }
}

ELEMENT **
parse_def (enum command_id command, ELEMENT *current)
{
  int contents_idx = 0;
  int type, set_type_not_arg;
  int i, i_def;
  int arg_types_nr;
  ELEMENT *e, *e1;
  ELEMENT **result;
  enum command_id *arguments_types_list;
  int inserted_category = 0;

  split_def_args (current, contents_idx);

  /* Check for "def alias" - for example @defun for @deffn. */
  if (command_data(command).flags & CF_def_alias)
    {
      char *category;
      int i;
      for (i = 0; def_aliases[i].alias ; i++)
        {
          if (def_aliases[i].alias == command)
            goto found;
        }
      fatal ("no alias for CF_def_alias command");
    found:
      /* Prepended content is inserted into contents, so
         @defun is converted into @deffn Function */

      category = def_aliases[i].category;
      command = def_aliases[i].command;

      inserted_category = 1;
      e = new_element (ET_def_line_arg);
      insert_into_contents (current, e, contents_idx);
      e1 = new_element (ET_NONE);
      text_append_n (&e1->text, category, strlen (category));
      add_to_element_contents (e, e1);
      if (global_documentlanguage && *global_documentlanguage)
        {
          e->type = ET_untranslated_def_line_arg;
          e1->type = ET_untranslated;
          add_extra_string_dup (e, "documentlanguage",
                                global_documentlanguage);
          if (def_aliases[i].translation_context)
            add_extra_string_dup (e, "translation_context",
                                  def_aliases[i].translation_context);
        }

      e = new_element (ET_spaces);
      text_append_n (&e->text, " ", 1);
      add_info_integer (e, "inserted", 1);
      insert_into_contents (current, e, contents_idx + 1);
    }

 /* Read arguments as CATEGORY [CLASS] [TYPE] NAME [ARGUMENTS]. */

  for (i_def = 0; i_def < sizeof (def_maps) / sizeof (*def_maps); i_def++)
    {
      if (def_maps[i_def].command == command)
        goto def_found;
    }
  fatal ("no arguments for def command");
 def_found:

  /* determine non arg/argtype number of arguments */
  arg_types_nr = 0;
  arguments_types_list = def_maps[i_def].argument_types;
  while (arguments_types_list[arg_types_nr])
    {
      enum element_type arg_type = arguments_types_list[arg_types_nr];

      if (arg_type == ET_def_typearg || arg_type == ET_def_arg)
        break;
      arg_types_nr++;
    }
  result = malloc ((arg_types_nr+1) * sizeof (ELEMENT *));

  for (i = 0; i < arg_types_nr; i++)
    {
      ELEMENT *e = next_bracketed_or_word_agg (current, &contents_idx);

      if (e)
        {
          enum element_type arg_type = arguments_types_list[i];
          ELEMENT *new_def_type = new_element (arg_type);

          new_def_type->parent = e->parent;
          current->contents.list[contents_idx - 1] = new_def_type;
          add_to_element_contents (new_def_type, e);
          result[i] = new_def_type;
        }
      else
        break;
    }

  result[i] = 0;

  if (inserted_category)
    {
      add_info_integer (current->contents.list[0], "inserted", 1);
    }

  /* Process args */
  split_delimiters (current, contents_idx);

  /* For some commands, alternate between "arg" and "typearg". This matters for
     the DocBook output. */
  /* In that case set_type_not_arg is both used to set to argtype and
     to switch sign to switch between arg and argtype */
  if (command == CM_deftypeline || command == CM_deftypefn
      || command == CM_deftypeop || command == CM_deftp)
    set_type_not_arg = -1;
  else
    set_type_not_arg = 1;

  type = set_type_not_arg;
  for (i = contents_idx; i < current->contents.number; i++)
    {
      enum element_type def_arg_type = ET_def_arg;
      e = contents_child_by_index (current, i);
      if (e->type == ET_spaces)
        {
          continue;
        }
      if (e->type == ET_delimiter)
        {
          type = set_type_not_arg;
          continue;
        }
      if (e->type == ET_def_line_arg && e->contents.number == 1
          && e->contents.list[0]->cmd && e->contents.list[0]->cmd != CM_code)
        {
          type = set_type_not_arg;
        }
      else
        {
          if (type != 1) {
            def_arg_type = ET_def_typearg;
          }
          type *= set_type_not_arg;
        }
      ELEMENT *new_def_type = new_element (def_arg_type);
      new_def_type->parent = e->parent;
      add_to_element_contents (new_def_type, e);
      current->contents.list[i] = new_def_type;
    }
  return result;
}
