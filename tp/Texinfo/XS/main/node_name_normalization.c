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

/* In sync with Texinfo::Convert::NodeNameNormalization */

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "unistr.h"
/*
#include "xvasprintf.h"
*/

/* for whitespace_chars, isascii_alnum and bug */
#include "utils.h"
#include "tree_types.h"
#include "tree.h"
#include "element_types.h"
/* for xasprintf */
#include "errors.h"
/* also for xvasprintf */
#include "text.h"
#include "extra.h"
#include "builtin_commands.h"
#include "debug.h"
#include "call_perl_function.h"
#include "unicode.h"
#include "node_name_normalization.h"

/* put in another file? Add an extern declaration in the .h file?*/
#include "cmd_normalization.c"

int ref_3_args_order[] = {0, 1, 2, -1};
int ref_5_args_order[] = {0, 1, 2, 4, 3, -1};

#define ADD(x) text_append (result, x)
void
convert_to_normalized_internal (const ELEMENT *e, TEXT *result)
{
  if ((e->type == ET_ignorable_spaces_after_command
       || e->type == ET_postamble_after_end
       || e->type == ET_preamble_before_beginning
       || e->type == ET_spaces_at_end
       || e->type == ET_spaces_before_paragraph
       || e->type == ET_space_at_end_menu_node
       || e->type == ET_spaces_after_close_brace)
      || (e->cmd
          && ((e->cmd == CM_anchor
               || e->cmd == CM_footnote
               || e->cmd == CM_shortcaption
               || e->cmd == CM_caption
               || e->cmd == CM_hyphenation
               || e->cmd == CM_sortas
               || e->cmd == CM_seealso
               || e->cmd == CM_seeentry)
             /* here ignore the line commands */
              || (e->args.number > 0
                  && (e->args.list[0]->type == ET_line_arg
                      || e->args.list[0]->type == ET_rawline_arg)))))
    return;
  else if (e->text.end > 0)
    {
      char *text_norm_spaces = collapse_spaces (e->text.text);
      ADD(text_norm_spaces);
      free (text_norm_spaces);
    }
  if (e->cmd)
    {
      if (command_normalization_text[e->cmd])
        ADD(command_normalization_text[e->cmd]);
      else if (e->cmd == CM_click)
        {
          enum command_id cmd;
          char *command_name = lookup_extra_string (e, "clickstyle");
          if (command_name)
            {
              cmd = lookup_builtin_command (command_name);
              if (command_normalization_text[cmd])
                ADD(command_normalization_text[cmd]);
            }
        }
      else if (builtin_command_data[e->cmd].flags & CF_accent)
        {
          if (e->args.number > 0)
            {
              TEXT accent_text;
              char *accented_char;

              text_init (&accent_text);
              convert_to_normalized_internal (e->args.list[0], &accent_text);
              accented_char = unicode_accent (accent_text.text, e);
              if (accented_char)
                {
                  ADD(accented_char);
                  free (accented_char);
                }
              else
     /* If this case was possible, the node normalization would not follow the
        specification, but it is not possible, see unicode_accent. */
                ADD(accent_text.text);
              free (accent_text.text);
            }
        }
      else if (builtin_command_data[e->cmd].flags & CF_ref)
        {
          int index = 0;
          int *arguments_order = ref_5_args_order;
          if (e->cmd == CM_inforef || e->cmd == CM_link)
            arguments_order = ref_3_args_order;
          while (arguments_order[index] >= 0)
            {
              if (e->args.number > arguments_order[index])
                {
                  TEXT arg_text;

                  text_init (&arg_text);
                  convert_to_normalized_internal (
                    e->args.list[arguments_order[index]], &arg_text);
                  if (arg_text.end > 0)
                    {
                      char *non_space_char = arg_text.text
                              + strspn (arg_text.text, whitespace_chars);
                      if (*non_space_char)
                        {
                          ADD (arg_text.text);
                          free (arg_text.text);
                          break;
                        }
                    }
                }
              index++;
            }
        }
      else if (e->args.number > 0
               && (e->args.list[0]->type == ET_brace_command_arg
                   || e->cmd == CM_math))
        {
          convert_to_normalized_internal (e->args.list[0], result);
          return;
        }
    }
  if (e->contents.number > 0)
    {
      int i;
      for (i = 0; i < e->contents.number; i++)
        convert_to_normalized_internal (e->contents.list[i], result);
    }
}
#undef ADD

/* Return value to be freed by caller. */
char *
convert_to_normalized (const ELEMENT *e)
{
  TEXT result;

  if (!e)
    return strdup ("");
  text_init (&result);
  /* this is needed for a test result with empty listoffloats */
  text_append (&result, "");
  convert_to_normalized_internal (e, &result);
  return result.text;
}

void
protect_unicode_char (const char *text, TEXT *result)
{
  uint8_t *encoded_u8;
  const uint8_t *next;
  ucs4_t next_char;
  char *str;

  /* determine unicode codepoint */
  encoded_u8 = utf8_from_string (text);
  next = u8_next (&next_char, encoded_u8);
  if (next && *next)
    bug ("Something left on next_str/encoded_u8\n");
  free (encoded_u8);

  if (next_char <= 0xFFFF)
    {
      xasprintf (&str, "%04lx", next_char);
      text_append_n (result, "_", 1);
    }
  else
    {
      xasprintf (&str, "%06lx", next_char);
      text_append_n (result, "__", 2);
    }
  text_append (result, str);
  free (str);
}

/* to be freed by caller */
char *unicode_to_protected (const char *text)
{
  TEXT result;
  const char *p = text;

  text_init (&result);
  text_append (&result, "");

  while (*p)
    {
      int n = strspn (p, " ");
      if (n)
        {
          text_append_n (&result, "-", 1);
          p += n;
          if (!*p)
            break;
        }

      if (isascii_alnum (*p))
        {
          text_append_n (&result, p, 1);
          p++;
        }
      else
        {
          int char_len = 1;
          char *next_str;

          /* Count any UTF-8 continuation bytes. */
          while ((p[char_len] & 0xC0) == 0x80)
            char_len++;

          next_str = strndup (p, char_len);
          protect_unicode_char (next_str, &result);

          free (next_str);
          p += char_len;
        }
    }
  return (result.text);
}

/* to be freed by caller */
char *normalize_top_name (const char *text)
{
  if (strlen (text) == 3)
    {
      char *normalized = strdup (text);
      char *p;

      for (p = normalized; *p; p++)
        if (isascii_alnum (*p))
          {
            *p = tolower (*p);
          }
        else
          {
            free (normalized);
            return strdup (text);
          }

      if (!strcmp (normalized, "top"))
        {
          free (normalized);
          return strdup ("Top");
        }

      free (normalized);
      return strdup (text);
    }
  return strdup (text);
}

/* to be freed by caller */
char *
convert_to_identifier (const ELEMENT *root)
{
  char *converted_name = convert_to_normalized (root);
  char *normalized_name = normalize_NFC (converted_name);
  char *protected = unicode_to_protected (normalized_name);
  char *result = normalize_top_name (protected);

  free (protected);
  free (converted_name);
  free (normalized_name);
  return result;
}

char *
convert_contents_to_identifier (const ELEMENT *e)
{
  ELEMENT *tmp = new_element (ET_NONE);
  char *result;

  tmp->contents = e->contents;
  result = convert_to_identifier (tmp);
  tmp->contents.list = 0;
  destroy_element (tmp);

  return result;
}

static char *
unicode_to_transliterate (char *text, int external)
{
  char *result;
  int status;
  if (external)
    result = call_nodenamenormalization_unicode_to_transliterate (text);
  else
    result = encode_string (text, "us-ascii//TRANSLIT", &status, 0);

  return result;
}

char *
normalize_transliterate_texinfo (const ELEMENT *e, int external_translit)
{
  char *converted_name = convert_to_normalized (e);
  char *normalized_name = normalize_NFC (converted_name);
  char *transliterated = unicode_to_transliterate (normalized_name,
                                                   external_translit);
  char *result = unicode_to_protected (transliterated);

  free (converted_name);
  free (normalized_name);
  free (transliterated);
  return result;
}

char *
normalize_transliterate_texinfo_contents (const ELEMENT *e,
                                          int external_translit)
{
  ELEMENT *tmp = new_element (ET_NONE);
  char *result;

  tmp->contents = e->contents;
  result = normalize_transliterate_texinfo (tmp, external_translit);
  tmp->contents.list = 0;
  destroy_element (tmp);

  return result;
}
