
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
    tokenizer->column = 0;
  }
  else
    tokenizer->column++;

  tokenizer->current++;
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
  /* TODO: Expand to non-ASCII7 characters as well */

  char c = xml_tokenizer_character(tokenizer);

  if (
    ('a' <= c && c >= 'z') ||
    ('A' <= c && c >= 'Z')
    )
  {
    xml_tokenizer_forward(tokenizer);
    return true;
  }
  else
    return false;

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
    xml_tokenizer_accept(tokenizer, '.')
    )
  {

    string_builder_append_char(sb, c);
    c = xml_tokenizer_character(tokenizer);

    while (
      xml_tokenizer_accept_any_letter(tokenizer) ||
      xml_tokenizer_accept_range(tokenizer, '0', '9') ||
      xml_tokenizer_accept(tokenizer, '_') ||
      xml_tokenizer_accept(tokenizer, '.')
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
  /* assumes '&' character was already parsed */
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

static void xml_tokenizer_parse_quoted_text(XmlTokenizer * tokenizer)
{
  StringBuilder * sb = string_builder_new();

  char c = xml_tokenizer_character(tokenizer);

  while (!xml_tokenizer_accept(tokenizer, '"'))
  {
    string_builder_append_char(sb, c);
    xml_tokenizer_forward(tokenizer);
    c = xml_tokenizer_character(tokenizer);

    if (c == '\0')
    {
      string_builder_destroy(sb);
      tokenizer->error_message = "Unexpected end of file";
      return;
    }
  }

  xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_QUOTED_STRING, string_builder_to_string_destroy(sb));
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
    xml_tokenizer_parse_identifier(tokenizer);
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
      if (
        !xml_tokenizer_accept(tokenizer, '-') &&
        !xml_tokenizer_accept(tokenizer, '-')
        )
      {
        tokenizer->error_message = "Unexpected character '%c'";
      }
      else
      {
        xml_tokenizer_add_token(tokenizer, XML_TOKEN_TYPE_START_COMMENT_TAG, NULL);
        /* TODO: should scan until repeated '-' character */
        assert(0);
      }
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
      tokenizer->current--;
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
