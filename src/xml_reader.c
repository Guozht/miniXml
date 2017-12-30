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

 

#include <assert.h>
#include <baselib/baselib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "xml_element.h"
#include "xml_element_struct.h"
#include "xml_tokenizer.h"
#include "xml_token.h"

#include "xml_reader.h"


struct XmlReader
{
  List * tokens;
  Dictionary * entities;
  unsigned int tokens_current;

  char * error_message;
  char * returnable_error_message;
};

/* INTERNALS */

static void xml_reader_set_error_message(XmlReader * reader, char * error)
{
  reader->error_message = strings_clone(error);
}

static void xml_reader_load_default_entities(XmlReader * reader)
{
    reader->entities = dictionary_new(DICTIONARY_TYPE_HASH_TABLE);

    dictionary_put(reader->entities, "lt", str_to_any("<"));
    dictionary_put(reader->entities, "gt", str_to_any(">"));
    dictionary_put(reader->entities, "quot", str_to_any("\""));
    dictionary_put(reader->entities, "apos", str_to_any("\'"));
    dictionary_put(reader->entities, "amp", str_to_any("&"));
}

static void xml_reader_unload_entities(XmlReader * reader)
{

  dictionary_remove(reader->entities, "lt"); /* remove as not to free static data */
  dictionary_remove(reader->entities, "gt");
  dictionary_remove(reader->entities, "quot");
  dictionary_remove(reader->entities, "apos");
  dictionary_remove(reader->entities, "amp");

  dictionary_destroy_and_free(reader->entities);
}

static uint32_t * xml_reader_find_declaration(uint32_t * data, size_t size, size_t * ret_size)
{
  uint32_t * ret;
  size_t ret_index;
  bool start_found = false;

  for (size_t k = 0; k < size - 1; k++)
  {
    if (!start_found)
    {
      if (
          data[k] == (uint32_t) '<' &&
          data[k + 1] == (uint32_t) '?'
          )
      {
        start_found = true;
        ret = &data[k + 2];
        ret_index = k + 2;
        k += 1;
      }
    }
    else
    {
      if (
          data[k] == (uint32_t) '?' &&
          data[k + 1] == (uint32_t) '>'
          )
      {
        *ret_size = k - ret_index;
        return ret;
      }
    }
  }

  return NULL;
}


static Dictionary * xml_reader_parse_declaration(
    uint32_t * declaration,
    size_t declaration_length,
    UnicodeEncodingType encoding_type
  )
{
  size_t declaration_string_length;
  char * declaration_string = unicode_write_string_utf8(
      declaration,
      declaration_length,
      &declaration_string_length
    );
  char * post_xml_string;
  Dictionary * ret;

  if (!strings_starts_with(declaration_string, "xml "))
  {
    free(declaration_string);
    return NULL;
  }

  post_xml_string = strings_suffix(declaration_string, strings_length(declaration_string) - 4);
  free(declaration_string);

  XmlTokenizer * tokenizer = xml_tokenizer_new(post_xml_string);
  LinkedList * list = xml_tokenizer_tokenize_declaration(tokenizer);

  free(post_xml_string);

  if (list == NULL || linked_list_size(list) % 3 != 0 || linked_list_size(list) == 0)
  {
    xml_tokenizer_destroy(tokenizer); /* FREES TOKEN LIST */
    return NULL;
  }

  ret = (Dictionary *) hash_table_new();

  for (unsigned int k = 0; k < linked_list_size(list); k += 3)
  {
    XmlToken
      * key = (XmlToken *) any_to_ptr(linked_list_get(list, k)),
      * equals = (XmlToken *) any_to_ptr(linked_list_get(list, k + 1)),
      * value = (XmlToken *) any_to_ptr(linked_list_get(list, k + 2));

    if (
        key->type != XML_TOKEN_TYPE_IDENTIFIER ||
        equals->type != XML_TOKEN_TYPE_EQUALS ||
        value->type != XML_TOKEN_TYPE_QUOTED_STRING
      )
    {
      xml_tokenizer_destroy(tokenizer); /* FREES TOKEN LIST */
      dictionary_destroy_and_free(ret);
      return NULL;
    }

    char * value_string = strings_clone(value->data);
    if (!dictionary_try_put(ret, key->data, str_to_any(value_string)))
    {
      xml_tokenizer_destroy(tokenizer); /* FREES TOKEN LIST */
      dictionary_destroy_and_free(ret);
      free(value_string);
      return NULL;
    }
  }

  xml_tokenizer_destroy(tokenizer); /* FREES TOKEN LIST */

  return ret;
}

static bool xml_reader_encoding_matches(Dictionary * dec, UnicodeEncodingType type)
{
  char
    * declared_encoding = any_to_str(dictionary_get(dec, "encoding")),
    * detected_encoding = unicode_encoding_type_to_string(type);

  if (strings_equals_ignore_case(declared_encoding, detected_encoding))
    return true;

  if (
    !strings_ends_with_ignore_case(detected_encoding, "BE") &&
    !strings_ends_with_ignore_case(detected_encoding, "LE")
    )
    return false;

  if (strings_length(detected_encoding) - 2 != strings_length(declared_encoding))
    return false;

  return strings_starts_with_ignore_case(detected_encoding, declared_encoding);
}

static Dictionary * xml_reader_parse_and_confirm_declaration(
    uint32_t * code_points,
    size_t code_points_length,
    UnicodeEncodingType type,
    uint32_t ** root_start,
    size_t * root_start_length
  )
{
  uint32_t * declaration;
  size_t declaration_length;

  declaration = xml_reader_find_declaration(
        code_points,
        code_points_length,
        &declaration_length
      );
  if (!declaration)
    return NULL;

  *root_start = &declaration[declaration_length + 2];
  *root_start_length = code_points_length - declaration_length - 2 -  (unsigned) (declaration - code_points) / sizeof(uint32_t);

  Dictionary * dic = xml_reader_parse_declaration(
      declaration,
      declaration_length,
      type
    );
  if (!dic)
  {
    return NULL;
  }

  /* if no encoding specified, assumes decodable is correct */
  if (!dictionary_has(dic, "encoding"))
    return dic;

  if (!xml_reader_encoding_matches(dic, type))
  {
    dictionary_destroy_and_free(dic);
    return NULL;
  }


  return dic;
}


static void xml_reader_reset(XmlReader * reader)
{
  if (reader->error_message)
    free(reader->error_message);
  if (reader->returnable_error_message)
    free(reader->returnable_error_message);

  reader->error_message = NULL;
  reader->returnable_error_message = NULL;
}


static XmlToken * xml_reader_current_token(XmlReader * reader)
{
  if (reader->tokens_current >= list_size(reader->tokens))
    return (XmlToken *) any_to_ptr(list_get(reader->tokens, list_size(reader->tokens) - 1));
  else
    return (XmlToken *) any_to_ptr(list_get(reader->tokens, reader->tokens_current));
}

static bool xml_reader_accept(XmlReader * reader, enum XmlTokenType type)
{
  XmlToken * token = xml_reader_current_token(reader);
  if (token->type == type)
  {
    reader->tokens_current++;
    return true;
  }
  else
    return false;
}

static void xml_reader_prepare_error_message(XmlReader * reader, bool with_token)
{
  if (reader->error_message)
  {
    if (with_token)
    {
      XmlToken * token = xml_reader_current_token(reader);
      char * token_string = xml_token_to_string(token);

      reader->returnable_error_message = strings_format("%s (%s)", reader->error_message, token_string);
      free(token_string);
    }
    else
    {
      reader->returnable_error_message = strings_clone(reader->error_message);
    }

  }
}


static XmlElement * xml_reader_parse_element_imp(XmlReader * reader, bool first)
{
  StringBuilder
    * attrib_sb;
  XmlElement
    * ret,
    * child;
  XmlToken
    * tag_name_token,
    * attrib_name_token,
    * attrib_value_token,
    * pre_token
    ;
  bool
    in_token_loop,
    in_attrib_loop,
    tag_empty;

  if (first)
  {
    /* clear valid whitespace */

    pre_token = xml_reader_current_token(reader);
    while (xml_reader_accept(reader, XML_TOKEN_TYPE_TEXT))
    {
      if (!strings_is_whitespace(pre_token->data))
      {
        xml_reader_set_error_message(reader, "Junk text before root");
        return NULL;
      }
      pre_token = xml_reader_current_token(reader);
    }
  }

  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_START_TAG))
  {
    xml_reader_set_error_message(reader, "Expected '<'");
    return NULL;
  }

  tag_name_token = xml_reader_current_token(reader);
  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_IDENTIFIER))
  {
    xml_reader_set_error_message(reader, "Expected tag name identifier");
    return NULL;
  }

  ret = xml_element_new(tag_name_token->data);


  in_token_loop = true;
  do
  {

    if (xml_reader_accept(reader, XML_TOKEN_TYPE_END_TAG))
    {
      tag_empty = false;
      in_token_loop = false;
      continue;
    }
    if (xml_reader_accept(reader, XML_TOKEN_TYPE_END_EMPTY_TAG))
    {
      tag_empty = true;
      in_token_loop = false;
      continue;
    }


    attrib_name_token = xml_reader_current_token(reader);

    if (!xml_reader_accept(reader, XML_TOKEN_TYPE_IDENTIFIER))
    {
      xml_reader_set_error_message(reader, "Expected attribute identifier");
      xml_element_destroy(ret);
      return NULL;
    }
    if (!xml_reader_accept(reader, XML_TOKEN_TYPE_EQUALS))
    {
      xml_reader_set_error_message(reader, "Expected '='");
      xml_element_destroy(ret);
      return NULL;
    }

    in_attrib_loop = true;
    attrib_sb = string_builder_new();
    while (in_attrib_loop)
    {
      attrib_value_token = xml_reader_current_token(reader);
      if (!xml_reader_accept(reader, XML_TOKEN_TYPE_QUOTED_STRING))
      {
        xml_reader_set_error_message(reader, "Expected quoted attribute value");
        xml_element_destroy(ret);
        string_builder_destroy(attrib_sb);
        return NULL;
      }
      string_builder_append(attrib_sb, attrib_value_token->data);

      attrib_value_token = xml_reader_current_token(reader);
      if (xml_reader_accept(reader, XML_TOKEN_TYPE_ENTITY))
      {
        Any value;
        if (!dictionary_try_get(reader->entities, attrib_value_token->data, &value))
        {
          reader->tokens_current--;
          xml_reader_set_error_message(reader, "Unknown entity");
          xml_element_destroy(ret);
          string_builder_destroy(attrib_sb);
          return NULL;
        }

        string_builder_append(attrib_sb, any_to_str(value));
      }
      else
        in_attrib_loop = false;

    }

    xml_element_add_attribute(
      ret,
      xml_attribute_new(
        attrib_name_token->data,
        string_builder_to_temp_string(attrib_sb)
        )
      );
  }
  while (in_token_loop);

  if (tag_empty)
    return ret;


  Any entity_value_any;
  StringBuilder * text_sb = string_builder_new();
  while (
    !xml_reader_accept(reader, XML_TOKEN_TYPE_START_END_TAG)
    )
  {

    XmlToken * current = xml_reader_current_token(reader);

    switch (current->type)
    {
      case XML_TOKEN_TYPE_TEXT:
        string_builder_append(text_sb, current->data);
        reader->tokens_current++;
        break;
      case XML_TOKEN_TYPE_ENTITY:
        if (!dictionary_try_get(reader->entities, current->data, &entity_value_any))
        {
          xml_reader_set_error_message(reader, "Unknown entity");
          xml_element_destroy(ret);
          string_builder_destroy(text_sb);
          return NULL;
        }
        string_builder_append(text_sb, any_to_str(entity_value_any));
        reader->tokens_current++;
        break;
      case XML_TOKEN_TYPE_START_TAG:
        array_list_add(ret->children, str_to_any(string_builder_to_string(text_sb)));
        string_builder_clear(text_sb);
        child = xml_reader_parse_element_imp(reader, false);
        if (!child)
        {
          xml_element_destroy(ret);
          return NULL;
        }
        array_list_add(ret->children, void_to_any(child));
        break;
      default:
        xml_reader_set_error_message(reader, "Unexpected token");
        xml_element_destroy(ret);
        string_builder_destroy(text_sb);
        return NULL;
    }
  }

  if (string_builder_length(text_sb))
  {
    array_list_add(ret->children, str_to_any(string_builder_to_string(text_sb)));
  }
  string_builder_destroy(text_sb);

  XmlToken * close_identifier = xml_reader_current_token(reader);
  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_IDENTIFIER))
  {
    xml_reader_set_error_message(reader, "Unexpected token");
    return NULL;
  }

  if (!strings_equals(close_identifier->data, tag_name_token->data))
  {
    reader->tokens_current--;
    xml_reader_set_error_message(reader, "Close tag does not match open tag");
    return NULL;
  }

  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_END_TAG))
  {
    xml_reader_set_error_message(reader, "Unexpected token");
    return NULL;
  }


  return ret;
}




XmlReader * xml_reader_new()
{
  XmlReader * reader = (XmlReader *) malloc(sizeof(XmlReader));
  assert(reader);

  reader->error_message = NULL;
  reader->returnable_error_message = NULL;

  return reader;
}

void xml_reader_destroy(XmlReader * reader)
{
  assert(reader);

  if (reader->error_message)
    free(reader->error_message);
  if (reader->returnable_error_message)
    free(reader->returnable_error_message);

  free(reader);
}


char * xml_reader_get_error_message(XmlReader * reader)
{
  assert(reader);

  return strings_clone(reader->returnable_error_message);
}

XmlDocument * xml_reader_parse_document(XmlReader * reader, char * data, size_t data_length)
{
  assert(reader);
  assert(data);
  assert(data_length > 0);

  size_t code_points_length, root_start_length;
  uint32_t * code_points, * root_start;
  Dictionary * declaration = NULL;
  bool declaration_found = false;

  int readable_types_count = 5;
  UnicodeEncodingType readable_types [readable_types_count];
  UnicodeEncodingType used_type = -1;
  bool well_formed [readable_types_count];
  readable_types[0] = UNICODE_ENCODING_TYPE_UTF8;
  readable_types[1] = UNICODE_ENCODING_TYPE_UTF16BE;
  readable_types[2] = UNICODE_ENCODING_TYPE_UTF16LE;
  readable_types[3] = UNICODE_ENCODING_TYPE_UTF32BE;
  readable_types[4] = UNICODE_ENCODING_TYPE_UTF32LE;

  for (unsigned int k = 0; k < readable_types_count; k++)
  {
    well_formed[k] = unicode_is_well_formed(readable_types[k], data, data_length);
    if (well_formed[k] && used_type == -1)
        used_type = readable_types[k];
  }

  if (used_type == -1)
  {
      xml_reader_set_error_message(reader, "Failed to determine valid encoding type");
      xml_reader_prepare_error_message(reader, false);
      return NULL;
  }

  for (unsigned int k = 0; k < readable_types_count && !declaration_found; k++)
  {
    if (well_formed[k])
    {
      code_points = unicode_read_string(readable_types[k], data, data_length, &code_points_length);
      declaration = xml_reader_parse_and_confirm_declaration(
          code_points,
          code_points_length,
          readable_types[k],
          &root_start,
          &root_start_length
        );

      if (declaration == NULL)
      {
        free(code_points);
        continue;
      }

      used_type = readable_types[k];
      declaration_found = true;
    }
  }

  if (!declaration_found)
  {
    code_points = unicode_read_string(used_type, data, data_length, &code_points_length);
    root_start = code_points;
    root_start_length = code_points_length;

  }

  size_t root_string_length;
  char * root_string = unicode_write_string_utf8(root_start, root_start_length, &root_string_length);
  free(code_points);

  XmlElement * root = xml_reader_parse_element(reader, root_string);
  if (!root)
  {
    if (declaration)
      dictionary_destroy_and_free(declaration);
    free(root_string);
    return NULL;
  }
  free(root_string);

  XmlDocument * ret = xml_document_new_with_root(root);
  xml_document_set_encoding(ret, unicode_encoding_type_to_string(used_type));
  if (declaration != NULL && dictionary_has(declaration, "version"))
    xml_document_set_version(ret, any_to_str(dictionary_get(declaration, "version")));

  if (declaration)
    dictionary_destroy_and_free(declaration);

  return ret;
}

XmlElement * xml_reader_parse_element(XmlReader * reader, char * data)
{
  assert(reader);
  assert(data);

  xml_reader_reset(reader);


  XmlTokenizer * tokenizer = xml_tokenizer_new(data);
  LinkedList * tokens = xml_tokenizer_tokenize(tokenizer);

  if (!tokens)
  {
    char buffer [1024];
    if (strings_contains_string(tokenizer->error_message, "%c"))
    {
      char * concat = strings_concat(tokenizer->error_message, " [%02x] (line: %d, column: %d)");
      char c = xml_tokenizer_character(tokenizer);
      sprintf(buffer, concat, c, c, tokenizer->line, tokenizer->column);
      free(concat);
    }
    else
      sprintf(buffer, "%s (line: %d, column: %d)", tokenizer->error_message, tokenizer->line, tokenizer->column);


    xml_reader_set_error_message(reader, buffer);
    xml_reader_prepare_error_message(reader, false);

    return NULL;
  }
  else
  {

    reader->tokens = (List *) tokens;
    reader->tokens_current = 0;

    xml_reader_load_default_entities(reader);

    /* Add end of file token */
    list_add(reader->tokens, ptr_to_any(xml_token_new(XML_TOKEN_TYPE_END_OF_FILE, NULL)));

    XmlElement * element = xml_reader_parse_element_imp(reader, true);

    xml_reader_prepare_error_message(reader, true);

    list_destroy_and_user_free(reader->tokens, (void (*)(void *)) xml_token_destroy);
    xml_reader_unload_entities(reader);

    return element;
  }

}
