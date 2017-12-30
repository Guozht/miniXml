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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml_attribute.h"
#include "xml_attribute_struct.h"
#include "xml_document.h"
#include "xml_document_struct.h"
#include "xml_element.h"
#include "xml_element_struct.h"


#include "xml_writer.h"



#define XML_WRITER_DEFAULT_INDENT_WIDTH 2



struct XmlWriter
{
  enum XmlWriterStyle style;
  unsigned int indent_depth;
  unsigned int indent_width;
};


enum XmlAppendableType
{
  XML_APPENDABLE_TYPE_STRING_BUILDER,
  XML_APPENDABLE_TYPE_STRING,
  XML_APPENDABLE_TYPE_STREAM,
};

struct XmlAppendable
{
  enum XmlAppendableType type;
  unsigned int limit;
  unsigned int length;
  void * ptr;
};


static struct XmlAppendable * xml_appendable_new(enum XmlAppendableType type, void * ptr, unsigned int limit)
{
  struct XmlAppendable * ret = (struct XmlAppendable *) malloc(sizeof(struct XmlAppendable));
  assert(ret);

  ret->type = type;
  ret->limit = limit;
  ret->length = 0;
  ret->ptr = ptr;

  return ret;
}

static void xml_appendable_append(struct XmlAppendable * appendable, char * string)
{
  switch (appendable->type)
  {

    case XML_APPENDABLE_TYPE_STRING_BUILDER:
      string_builder_append((StringBuilder *) appendable->ptr, string);
      break;
    case XML_APPENDABLE_TYPE_STRING:
      if (appendable->length >= appendable->limit && appendable->limit != -1)
        return;

      unsigned int string_length = strings_length(string);
      if (appendable->length + string_length > appendable->limit && appendable->limit != -1)
      {
        memcpy(
          &((char *) appendable->ptr)[appendable->length],
          string,
          sizeof(char) * (string_length - appendable->limit + appendable->length)
          );
        appendable->length = appendable->limit;
      }
      else
      {
        memcpy(
          &((char *) appendable->ptr)[appendable->length],
          string,
          string_length
          );
        appendable->length += string_length;
      }
      ((char *) appendable->ptr)[appendable->length] = '\0';
      break;
    case XML_APPENDABLE_TYPE_STREAM:
      fputs(string, (FILE *) appendable->ptr);
      break;

    default:
      assert(0);

  }
}

static void xml_appendable_append_char(struct XmlAppendable * appendable, char c)
{
  switch (appendable->type)
  {

    case XML_APPENDABLE_TYPE_STRING_BUILDER:
      string_builder_append_char((StringBuilder *) appendable->ptr, c);
      break;
    case XML_APPENDABLE_TYPE_STRING:
      if (appendable->length >= appendable->limit && appendable->limit != -1)
        return;

      ((char *) appendable->ptr)[appendable->length++] = c;
      ((char *) appendable->ptr)[appendable->length] = '\0';

      break;
    case XML_APPENDABLE_TYPE_STREAM:
      fputc(c, (FILE *) appendable->ptr);
      break;

    default:
      assert(0);

  }
}



static char * xml_writer_escape_string(char * str, bool attribute)
{
  StringBuilder * sb = string_builder_new();
  unsigned int top = 0;

  char c;
  while (str[top] != '\0')
  {
    c = str[top++];

    switch (c)
    {

      case '&':
        string_builder_append(sb, "&amp;");
        break;
      case '<':
        string_builder_append(sb, "&lt;");
        break;
      case '>':
        string_builder_append(sb, "&gt;");
        break;
      case '\"':
        if (attribute)
          string_builder_append(sb, "&quot;");
        else
          string_builder_append_char(sb, '\"');
        break;
      case '\'':
        if (attribute)
          string_builder_append(sb, "&apos;");
        else
          string_builder_append_char(sb, '\'');
        break;

      default:
        string_builder_append_char(sb, c);
        break;

    }

  }

  return string_builder_to_string_destroy(sb);
}

static void xml_writer_print_indent_to_appendable(XmlWriter * writer, struct XmlAppendable * appendable)
{
  if (writer->style == XML_WRITER_STYLE_COMPRESSED)
    return;

  for (unsigned int k = 0; k < writer->indent_depth * writer->indent_width; k++)
  {
    xml_appendable_append_char(appendable, ' ');
  }
}

static void xml_writer_standard_write_attribute_callback(struct XmlAppendable * appendable, XmlAttribute * attribute)
{
  char * escaped_value;

  xml_appendable_append_char(appendable, ' ');
  xml_appendable_append(appendable, attribute->name);
  xml_appendable_append_char(appendable, '=');
  xml_appendable_append_char(appendable, '\"');

  escaped_value = xml_writer_escape_string(attribute->value, true);
  xml_appendable_append(appendable, escaped_value);
  free(escaped_value);

  xml_appendable_append_char(appendable, '\"');
}


static void xml_writer_write_element_attributes_to_appendable(XmlWriter * writer, struct XmlAppendable * appendable, XmlElement * element)
{
  List * attributes = xml_element_get_attributes(element);

  void (*writer_callback)(struct XmlAppendable *, XmlAttribute *);

  switch (writer->style)
  {

    case XML_WRITER_STYLE_COMPRESSED:
    case XML_WRITER_STYLE_STANDARD:
      writer_callback = xml_writer_standard_write_attribute_callback;
      break;
    case XML_WRITER_STYLE_EXPANDED:
      assert(0); /* NOT YET IMPLEMENTED */
      break;

    default:
      assert(0);

  }


  ListTraversal * traversal = list_get_traversal(attributes);
  while (!list_traversal_completed(traversal))
  {
    XmlAttribute * attribute = (XmlAttribute *) any_to_ptr(list_traversal_next(traversal));
    writer_callback(appendable, attribute);
  }

  list_destroy(attributes);
}


static void xml_writer_write_element_to_appendable(XmlWriter * writer, struct XmlAppendable * appendable, XmlElement * element)
{
  xml_appendable_append_char(appendable, '<');
  xml_appendable_append(appendable, element->name);

  char
    * empty_tag_end,
    * standard_tag_end,
    * close_tag_start;

  switch (writer->style)
  {
    case XML_WRITER_STYLE_STANDARD:
    case XML_WRITER_STYLE_EXPANDED:
      empty_tag_end = " />\n";
      standard_tag_end = ">\n";
      close_tag_start = "</";
      break;
    case XML_WRITER_STYLE_COMPRESSED:
      empty_tag_end = "/>";
      standard_tag_end = ">";
      close_tag_start = "</";
      break;
    default:
      assert(0);
  }

  xml_writer_write_element_attributes_to_appendable(writer, appendable, element);

  if (xml_element_is_empty(element))
  {
    xml_appendable_append(appendable, empty_tag_end);
  }
  else
  {

    if (array_list_size(element->children) == 1 && array_list_get(element->children, 0).type == ANY_TYPE_STRING)
    {
      xml_appendable_append_char(appendable, '>');
      char * string = xml_writer_escape_string(any_to_string(array_list_get(element->children, 0)), false);
      xml_appendable_append(appendable, string);
      free(string);
    }
    else
    {
      xml_appendable_append(appendable, standard_tag_end);

      writer->indent_depth++;

      bool lastWasText;
      ArrayListTraversal * traversal = array_list_get_traversal(element->children);
      while (!array_list_traversal_completed(traversal))
      {
        Any child_any = array_list_traversal_next(traversal);

        if (child_any.type == ANY_TYPE_POINTER)
        {
          XmlElement * child = (XmlElement *) any_to_ptr(child_any);
          xml_writer_print_indent_to_appendable(writer, appendable);
          xml_writer_write_element_to_appendable(writer, appendable, child);

          lastWasText = false;
        }
        else if (child_any.type == ANY_TYPE_STRING)
        {
          char * string = xml_writer_escape_string(any_to_string(child_any), false);
          xml_appendable_append(appendable, string);
          free(string);

          lastWasText = true;
        }
        else
          assert(0);
      }

      if (lastWasText && writer->style != XML_WRITER_STYLE_COMPRESSED)
        xml_appendable_append_char(appendable, '\n');

      writer->indent_depth--;
      xml_writer_print_indent_to_appendable(writer, appendable);
    }

    xml_appendable_append(appendable, close_tag_start);
    xml_appendable_append(appendable, element->name);
    xml_appendable_append(appendable, standard_tag_end);
  }
}





XmlWriter * xml_writer_new()
{
  XmlWriter * ret = (XmlWriter *) malloc(sizeof(XmlWriter));
  assert(ret);

  ret->style = XML_WRITER_STYLE_STANDARD;
  ret->indent_depth = 0;
  ret->indent_width = XML_WRITER_DEFAULT_INDENT_WIDTH;

  return ret;
}

void xml_writer_destroy(XmlWriter * writer)
{
  assert(writer);

  free(writer);
}



enum XmlWriterStyle xml_writer_get_style(XmlWriter * writer)
{
  assert(writer);

  return writer->style;
}

unsigned int xml_writer_get_indent_width(XmlWriter * writer)
{
  assert(writer);

  return writer->indent_width;
}

void xml_writer_set_style(XmlWriter * writer, enum XmlWriterStyle style)
{
  assert(writer);

  writer->style = style;
}

void xml_writer_set_indent_width(XmlWriter * writer, unsigned int width)
{
  assert(writer);

  writer->indent_width = width;
}


char * xml_writer_get_document_text(XmlWriter * writer, XmlDocument * document)
{
  assert(writer);
  assert(document);

  StringBuilder * sb = string_builder_new();

  /* HEADER */
  char header_string [1024];
  sprintf(
    header_string,
    "<?xml version=\"%s\" encoding=\"%s\"?>",
    document->version,
    document->encoding
    );
  string_builder_append(sb, header_string);
  string_builder_append_char(sb, '\n');

  /* ROOT ELEMENT */
  struct XmlAppendable * appendable = xml_appendable_new(XML_APPENDABLE_TYPE_STRING_BUILDER, (void *) string_builder_new(), -1);

  xml_writer_write_element_to_appendable(writer, appendable, xml_document_get_root(document));

  char * string = string_builder_to_string((StringBuilder *) appendable->ptr);
  string_builder_destroy((StringBuilder *) appendable->ptr);
  free(appendable);

  return string;
}

char * xml_writer_get_element_text(XmlWriter * writer, XmlElement * element)
{
  assert(writer);
  assert(element);

  struct XmlAppendable * appendable = xml_appendable_new(
      XML_APPENDABLE_TYPE_STRING_BUILDER,
      (void *) string_builder_new(),
      -1);

  xml_writer_write_element_to_appendable(writer, appendable, element);

  char * ret = string_builder_to_string((StringBuilder *) appendable->ptr);
  string_builder_destroy((StringBuilder *) appendable->ptr);
  free(appendable);

  return ret;
}


void xml_writer_print_document(XmlWriter * writer, XmlDocument * document, char * string)
{
  xml_writer_printn_document(writer, document, string, -1);
}

void xml_writer_printn_document(XmlWriter * writer, XmlDocument * document, char * string, unsigned int n)
{
  assert(writer);
  assert(document);
  assert(string);
  assert(n > 0 || n == -1);

  char header_string [1024];
  sprintf(
    header_string,
    "<?xml version=\"%s\" encoding=\"%s\"?>\n",
    document->version,
    document->encoding
    );

  unsigned int header_string_length = strings_length(header_string);
  memcpy(string, header_string, header_string_length);
  string[header_string_length] = '\0';

  struct XmlAppendable * appendable = xml_appendable_new(
      XML_APPENDABLE_TYPE_STRING,
      (void *) string,
      n);
  appendable->length = header_string_length;

  xml_writer_write_element_to_appendable(writer, appendable, xml_document_get_root(document));
  free(appendable);
}

void xml_writer_print_element(XmlWriter * writer, XmlElement * element, char * string)
{
  xml_writer_printn_element(writer, element, string, -1);
}

void xml_writer_printn_element(XmlWriter * writer, XmlElement * element, char * string, unsigned int n)
{
  assert(writer);
  assert(element);
  assert(string);
  assert(n > 0 || n != -1);

  struct XmlAppendable * appendable = xml_appendable_new(
      XML_APPENDABLE_TYPE_STRING,
      (void *) string,
      n);

  xml_writer_write_element_to_appendable(writer, appendable, element);
  free(appendable);
}


void xml_writer_write_document(XmlWriter * writer, XmlDocument * document, FILE * stream)
{
  assert(writer);
  assert(document);
  assert(stream);

  fprintf(
    stream,
    "<?xml version=\"%s\" encoding=\"%s\"?>\n",
    document->version,
    document->encoding
    );

  struct XmlAppendable * appendable = xml_appendable_new(
    XML_APPENDABLE_TYPE_STREAM,
    (void *) stream,
    -1);

  xml_writer_write_element_to_appendable(writer, appendable, xml_document_get_root(document));

  free(appendable);
}

void xml_writer_write_element(XmlWriter * writer, XmlElement * element, FILE * stream)
{
  assert(writer);
  assert(element);
  assert(stream);

  struct XmlAppendable * appendable = xml_appendable_new(
    XML_APPENDABLE_TYPE_STREAM,
    (void *) stream,
    -1);

  xml_writer_write_element_to_appendable(writer, appendable, element);

  free(appendable);
}
