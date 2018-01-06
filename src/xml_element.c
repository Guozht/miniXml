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
#include <stdlib.h>
#include <baselib/baselib.h>

#include "xml_attribute.h"
#include "xml_attribute_struct.h"
#include "xml_writer.h"
#include "xml_reader.h"

#include "xml_element.h"
#include "xml_element_struct.h"


static void xml_element_destroy_attribute_any(Any any)
{
  xml_attribute_destroy((XmlAttribute *) any_to_ptr(any));
}

static void xml_element_destroy_child_any(Any any)
{
  if (any.type == ANY_TYPE_STRING)
    free(any_to_string(any));
  else
    xml_element_destroy((XmlElement *) any_to_ptr(any));
}




XmlElement * xml_element_new(char * name)
{
  assert(name);

  XmlElement * ret = (XmlElement *) malloc(sizeof(XmlElement));
  assert(ret);

  ret->name = strings_clone(name);
  ret->attributes = array_list_new();
  ret->children = array_list_new();

  return ret;
}

XmlElement * xml_element_parse(char * string)
{
  assert(string);

  XmlReader * reader = xml_reader_new();
  XmlElement * ret = xml_reader_parse_element(reader, string);
  xml_reader_destroy(reader);

  return ret;
}


void xml_element_destroy(XmlElement * element)
{
  assert(element);

  array_list_destroy_and(element->attributes, xml_element_destroy_attribute_any);
  array_list_destroy_and(element->children, xml_element_destroy_child_any);
  free(element->name);

  free(element);
}



char * xml_element_get_name(XmlElement * element)
{
  assert(element);

  return strings_clone(element->name);
}

List * xml_element_get_attributes(XmlElement * element)
{
  assert(element);

  return (List *) array_list_clone(element->attributes);
}

List * xml_element_get_children(XmlElement * element)
{
  assert(element);

  return (List *) array_list_clone(element->children);
}

bool xml_element_is_empty(XmlElement * element)
{
  assert(element);

  return array_list_size(element->children) == 0;
}


XmlAttribute * xml_element_get_attribute(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  XmlAttribute * attrib;
  ArrayListTraversal * traversal = array_list_get_traversal(element->attributes);
  while (!array_list_traversal_completed(traversal))
  {

    attrib = (XmlAttribute *) any_to_ptr(array_list_traversal_next(traversal));

    if (strings_equals(attrib->name, name))
    {
      array_list_traversal_destroy(traversal);
      return attrib;
    }

  }

  return NULL;
}

XmlElement * xml_element_get_child(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  Any child_any;
  XmlElement * child;
  ArrayListTraversal * traversal = array_list_get_traversal(element->children);
  while (!array_list_traversal_completed(traversal))
  {
    child_any = array_list_traversal_next(traversal);
    if (child_any.type != ANY_TYPE_POINTER)
      continue;
    child = (XmlElement *) any_to_ptr(child_any);

    if (strings_equals(child->name, name))
    {
      array_list_traversal_destroy(traversal);

      return child;
    }
  }

  return NULL;
}

List * xml_element_get_children_by_name(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  List * ret = list_new(LIST_TYPE_LINKED_LIST);
  ArrayListTraversal * traversal;
  XmlElement * child;

  traversal = array_list_get_traversal(element->children);

  while (!array_list_traversal_completed(traversal))
  {
    Any child_any = array_list_traversal_next(traversal);
    if (child_any.type != ANY_TYPE_POINTER)
      continue;

    child = (XmlElement *) any_to_ptr(child_any);

    if (strings_equals(child->name, name))
      list_add(ret, child_any);
  }

  return ret;
}

char * xml_element_get_value(XmlElement * element)
{
  assert(element);

  XmlWriter * writer;
  ArrayListTraversal * traversal;
  StringBuilder * sb;

  traversal = array_list_get_traversal(element->children);
  sb = string_builder_new();

  writer = xml_writer_new();
  xml_writer_set_style(writer, XML_WRITER_STYLE_COMPRESSED);

  while (!array_list_traversal_completed(traversal))
  {
    Any child_any = array_list_traversal_next(traversal);
    if (child_any.type == ANY_TYPE_STRING)
    {
      string_builder_append(sb, any_to_string(child_any));
    }
    else if (child_any.type == ANY_TYPE_POINTER)
    {
      char * string = xml_writer_get_element_text(writer, (XmlElement *) any_to_ptr(child_any));
      string_builder_append(sb, string);
      free(string);
    }
    else
      assert(0);
  }

  xml_writer_destroy(writer);

  return string_builder_to_string_destroy(sb);
}

void xml_element_set_name(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  element->name = strings_clone(name);
}

void xml_element_set_value(XmlElement * element, char * value)
{
  array_list_clear_and(element->children, xml_element_destroy_child_any);
  array_list_add(element->children, string_to_any(strings_clone(value)));
}


void xml_element_clear_attributes(XmlElement * element)
{
  assert(element);

  array_list_clear_and(element->attributes, xml_element_destroy_attribute_any);
}

void xml_element_clear_children(XmlElement * element)
{
  assert(element);

  array_list_clear_and(element->attributes, xml_element_destroy_child_any);
}


void xml_element_add_attribute(XmlElement * element, XmlAttribute * attribute)
{
  assert(element);
  assert(attribute);

  array_list_add(element->attributes, ptr_to_any(attribute));
}

void xml_element_add_child(XmlElement * element, XmlElement * child)
{
  assert(element);
  assert(child);

  array_list_add(element->children, ptr_to_any(child));
}


void xml_element_add_attributes(XmlElement * element, List * attributes)
{
  assert(element);
  assert(attributes);

  array_list_add_range(element->attributes, attributes);
}

void xml_element_add_children(XmlElement * element, List * children)
{
  assert(element);
  assert(children);

  array_list_add_range(element->children, children);
}
