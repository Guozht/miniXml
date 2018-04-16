/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  miniXml: a simple XML parsing library for C                            *
 *  Copyright (C) 2017  LeqxLeqx                                           *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "xml_tokenizer.h"

#include <assert.h>
#include <baselib/baselib.h>
#include <stdbool.h>
#include <stdlib.h>


#include "xml_token.h"


/*
 * TODO: Add in Unicode Thingys
 *
 * Valids:   #x9 | #xA | #xD | [#x20-#xD7FF] |
 *          [#xE000-#xFFFD] | [#x10000-#x10FFFF]
 * (any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.)
 *
 */




/* internals */



static void xml_tokenizer_forward(XmlTokenizer * tokenizer)
{
  if (xml_tokenizer_character(tokenizer) == '\n')
  {
    tokenizer->line++;
    tokenizer->last_columnt_length = tokenizer->column;
    tokenizer->column = 0;
  }
  else
    tokenizer->column++;

  tokenizer->current++;
}

static void xml_tokenizer_backward(XmlTokenizer * tokenizer)
{
  tokenizer->current--;

  if (tokenizer->column == 0)
  {
    tokenizer->column = tokenizer->last_columnt_length;
    tokenizer->line--;
  }
  else
    tokenizer->column--;
}

static void xml_tokenizer_backward_n(XmlTokenizer * tokenizer, unsigned int n)
{
  /* TODO: MAKE NON-STUPID */
  for (unsigned int k = 0; k < n; k++)
  {
    xml_tokenizer_backward(tokenizer);
  }
}


static bool xml_tokenizer_accept(XmlTokenizer * tokenizer, char c)
{
  if (xml_tokenizer_character(tokenizer) == c)
  {
    xml_tokenizer_forward(tokenizer);
    return true;
  }
  else
    return false;
}

/* INCLUSIVE */
static bool xml_tokenizer_accept_range(XmlTokenizer * tokenizer, char start, char end)
{
  char c = xml_tokenizer_character(tokenizer);

  if (c >= start && c <= end)
  {
    xml_tokenizer_forward(tokenizer);
    return true;
  }
  else
    return false;

}

static bool xml_tokenizer_accept_whitespace(XmlTokenizer * tokenizer)
{
  char c = xml_tokenizer_character(tokenizer);

  if (chars_is_white_space(c) && c != 0)
  {
    xml_tokenizer_forward(tokenizer);
    return true;
  }
  else
    return false;
}

static bool xml_tokenizer_accept_any_letter(XmlTokenizer * tokenizer)
{

  char c = xml_tokenizer_character(tokenizer);

  if (
    ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    c > 0x7F /* considers all values out of ascii-7 to be valid characters */
    )
  {
    xml_tokenizer_forward(tokenizer);
    return true;
  }
  else
    return false;

}

static bool xml_tokenizer_accept_string(XmlTokenizer * tokenizer, char * string)
{
  unsigned int string_length = strings_length(string);

  for (unsigned int k = 0; k < string_length; k++)
  {
    if (!xml_tokenizer_accept(tokenizer, string[k]))
    {
      xml_tokenizer_backward_n(tokenizer, k);
      return false;
    }
  }

  return true;
}

static bool xml_tokenizer_expect_string(XmlTokenizer * tokenizer, char * string)
{
  unsigned int string_length = strings_length(string);

  for (unsigned int k = 0; k < string_length; k++)
  {
    if (!xml_tokenizer_accept(tokenizer, string[k]))
    {
      tokenizer->error_message = "Unexpected character '%c'";
      return false;
    }
  }

  return true;
}

static void xml_tokenizer_add_token(XmlTokenizer * tokenizer, enum XmlTokenType type, char * data)
{
  linked_list_add(tokenizer->tokens, ptr_to_any(xml_token_new(type, data)));
}

static void xml_tokenizer_parse_identifier(XmlTokenizer * tokenizer)
{

  char c = xml_tokenizer_character(tokenizer);
  StringBuilder * sb = string_builder_new();

  if (
    xml_tokenizer_accept_any_letter(tokenizer) ||
    xml_tokenizer_accept(tokenizer, '_') ||
    xml_tokenizer_accept(tokenizer, '.') ||
    xml_tokenizer_accept(tokenizer, ':')
    )
  {

    string_builder_append_char(sb, c);
    c = xml_tokenizer_character(tokenizer);

    while (
      xml_tokenizer_accept_any_letter(tokenizer) ||
      xml_tokenizer_accept_range(tokenizer, '0', '9') ||
      xml_tokenizer_accept(tokenizer, '-') ||
      xml_tokenizer_accept(tokenizer, '_') ||
      xml_tokenizer_accept(tokenizer, '.') ||
      xml_tokenizer_accept(tokenizer, ':')
      )
    {
      string_builder_append_char(sb, c);
      c = xml_tokenizer_character(tokenizer);
    }

    xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_IDENTIFIER, string_builder_to_string(sb));
  }

  string_builder_destroy(sb);
}

static void xml_tokenizer_parse_entity(XmlTokenizer * tokenizer)
{
  xml_tokenizer_forward(tokenizer); /* removing '&' character */
  StringBuilder * sb = string_builder_new();

  while (!xml_tokenizer_accept(tokenizer, ';'))
  {
    char c = xml_tokenizer_character(tokenizer);
    if (c == '\0')
    {
      tokenizer->error_message = "Unexpected: EOF while parsing XML entity";
      return;
    }
    string_builder_append_char(sb, c);
    xml_tokenizer_forward(tokenizer);
  }

  xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_ENTITY, string_builder_to_string_destroy(sb));
}

static void xml_tokenizer_parse_non_tag_data(XmlTokenizer * tokenizer)
{

  bool tag_met = false;
  StringBuilder * sb = string_builder_new();

  do
  {

    char c = xml_tokenizer_character(tokenizer);

    switch (c)
    {
      case '>':
        tokenizer->error_message = "Unexpected character: '%c'";
        break;

      case '&':
        if (string_builder_length(sb) > 0)
        {
          xml_tokenizer_add_token(
              tokenizer,
              XML_TOKEN_TYPE_TEXT,
              string_builder_to_string(sb)
            );
          string_builder_clear(sb);
        }
        xml_tokenizer_parse_entity(tokenizer);
        break;

      case '<':
        tag_met = true;
        break;

      case '\0': /* END OF FILE (EOF) */
        tag_met = true;
        break;

      default:
        string_builder_append_char(sb, c);
        xml_tokenizer_forward(tokenizer);
        break;
    }
  }
  while (!tag_met && !tokenizer->error_message);

  char * remnant = string_builder_to_string_destroy(sb);
  if (!strings_is_empty(remnant))
  {
    xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_TEXT, remnant);
  }

  free(remnant);
}

static void xml_tokenizer_parse_until_end_of_comment(XmlTokenizer * tokenizer)
{
  bool inside = true;

  do
  {
    if (xml_tokenizer_accept_string(tokenizer, "--"))
    {
      if (!xml_tokenizer_accept(tokenizer, '>'))
        tokenizer->error_message = "Unexpected '--' sequence within comment";

      inside = false;
    }
    else if (xml_tokenizer_accept(tokenizer, '\0'))
    {
      tokenizer->error_message = "Encountered EOF while parsing comment";
      inside = false;
    }
    else
      xml_tokenizer_forward(tokenizer);
  }
  while (inside);
}

static void xml_tokenizer_parse_quoted_text(XmlTokenizer * tokenizer)
{
  StringBuilder * sb = string_builder_new();

  char c = xml_tokenizer_character(tokenizer);

  while (!xml_tokenizer_accept(tokenizer, '"'))
  {
    string_builder_append_char(sb, c);
    xml_tokenizer_forward(tokenizer);
    c = xml_tokenizer_character(tokenizer);

    if (c == '&')
    {
      if (string_builder_length(sb) > 0)
      {
        xml_tokenizer_add_token(
            tokenizer,
            XML_TOKEN_TYPE_QUOTED_STRING,
            string_builder_to_string(sb)
          );
        string_builder_clear(sb);
      }
      xml_tokenizer_parse_entity(tokenizer);
      if (tokenizer->error_message)
        return;
      c = xml_tokenizer_character(tokenizer);
    }

    if (c == '\0')
    {
      string_builder_destroy(sb);
      tokenizer->error_message = "Unexpected end of file";
      return;
    }
  }

  xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_QUOTED_STRING, string_builder_to_string_destroy(sb));
}

static void xml_tokenizer_parse_cdata_segment(XmlTokenizer * tokenizer)
{
  StringBuilder * sb = string_builder_new();
  bool in_cdata_segment = true;

  while (!xml_tokenizer_accept(tokenizer, '\0') && in_cdata_segment)
  {
    if (xml_tokenizer_accept_string(tokenizer, "]]>"))
      in_cdata_segment = false;
    else
    {
      string_builder_append_char(sb, xml_tokenizer_character(tokenizer));
      xml_tokenizer_forward(tokenizer);
    }
  }

  xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_TEXT, string_builder_to_string_destroy(sb));
}

static void xml_tokenizer_parse_in_tag(XmlTokenizer * tokenizer, bool * in_tag)
{

  while (xml_tokenizer_accept_whitespace(tokenizer));

  if (xml_tokenizer_accept(tokenizer, '/'))
  {
    *in_tag = false;

    if (xml_tokenizer_accept(tokenizer, '>'))
      xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_END_EMPTY_TAG, NULL);
    else
      tokenizer->error_message = "Unexpected character '%c'";
  }
  else if (xml_tokenizer_accept(tokenizer, '>'))
  {
    *in_tag = false;

    xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_END_TAG, NULL);
  }
  else if (xml_tokenizer_accept(tokenizer, '='))
  {
    xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_EQUALS, NULL);
  }
  else if (xml_tokenizer_accept(tokenizer, '"'))
  {
    xml_tokenizer_parse_quoted_text(tokenizer);
  }
  else if (xml_tokenizer_accept_any_letter(tokenizer) || xml_tokenizer_accept(tokenizer, '_'))
  {
    xml_tokenizer_backward(tokenizer);
    xml_tokenizer_parse_identifier(tokenizer);
  }
  else if (xml_tokenizer_accept(tokenizer, '<'))
  {
    if (xml_tokenizer_expect_string(tokenizer, "!--"))
      xml_tokenizer_parse_until_end_of_comment(tokenizer);
  }
  else
  {
    tokenizer->error_message = "Unexpected character '%c'";
  }
}

static void xml_tokenizer_parse_out_tag(XmlTokenizer * tokenizer, bool * in_tag)
{
  if (xml_tokenizer_accept(tokenizer, '<'))
  {
    *in_tag = true;

    if (xml_tokenizer_accept(tokenizer, '/'))
    {
      xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_START_END_TAG, NULL);
      xml_tokenizer_parse_identifier(tokenizer);
    }
    else if (xml_tokenizer_accept(tokenizer, '!'))
    {
      /* TODO: ADD OTHER TYPES OF <! STATEMENTS */

      *in_tag = false;

      if (xml_tokenizer_accept(tokenizer, '-'))
      {
        if (!xml_tokenizer_accept(tokenizer, '-'))
          tokenizer->error_message = "Unexpected character '%c'";
        else
          xml_tokenizer_parse_until_end_of_comment(tokenizer);

      }
      else if (xml_tokenizer_accept(tokenizer, '['))
      {
        if (xml_tokenizer_expect_string(tokenizer, "CDATA["))
          xml_tokenizer_parse_cdata_segment(tokenizer);
      }
      else
        tokenizer->error_message = "Unexpected character '%c'";
    }
    else
    {
      xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_START_TAG, NULL);
      xml_tokenizer_parse_identifier(tokenizer);
    }
  }
  else
  {
    xml_tokenizer_parse_non_tag_data(tokenizer);
  }

}

static void xml_tokenizer_destroy_tokens_callback(Any any)
{
  xml_token_destroy((XmlToken *) any_to_ptr(any));
}



char xml_tokenizer_character(XmlTokenizer * tokenizer)
{
  if (tokenizer->current >= tokenizer->string_length)
    return '\0';
  else
    return tokenizer->string[tokenizer->current];
}


XmlTokenizer * xml_tokenizer_new(char * string)
{
  assert(string);

  XmlTokenizer * ret = (XmlTokenizer *) malloc(sizeof(XmlTokenizer));

  ret->string = string;
  ret->error_message = NULL;
  ret->string_length = strings_length(string);
  ret->current = 0;
  ret->line = 0;
  ret->column = 0;
  ret->tokens = linked_list_new();

  return ret;
}

void xml_tokenizer_destroy(XmlTokenizer * tokenizer)
{
  linked_list_destroy_and(tokenizer->tokens, xml_tokenizer_destroy_tokens_callback);
}

LinkedList * xml_tokenizer_tokenize(XmlTokenizer * tokenizer)
{

  bool in_tag = false;

  while (tokenizer->current < tokenizer->string_length)
  {
    if (in_tag)
      xml_tokenizer_parse_in_tag(tokenizer, &in_tag);
    else
      xml_tokenizer_parse_out_tag(tokenizer, &in_tag);

    if (tokenizer->error_message)
      return NULL;

  }

  return tokenizer->tokens;
}

LinkedList * xml_tokenizer_tokenize_declaration(XmlTokenizer * tokenizer)
{

  while (tokenizer->current < tokenizer->string_length)
  {
    while (xml_tokenizer_accept_whitespace(tokenizer));

    if (tokenizer->current >= tokenizer->string_length)
      continue;

    if (xml_tokenizer_accept_any_letter(tokenizer) || xml_tokenizer_accept(tokenizer, '_'))
    {
      xml_tokenizer_backward(tokenizer);
      xml_tokenizer_parse_identifier(tokenizer);
    }
    else if (xml_tokenizer_accept(tokenizer, '='))
      xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_EQUALS, NULL);
    else if (xml_tokenizer_accept(tokenizer, '\"'))
      xml_tokenizer_parse_quoted_text(tokenizer);
    else
      tokenizer->error_message = "Unexpected character: %c";

    if (tokenizer->error_message)
      return NULL;
  }

  return tokenizer->tokens;
}
