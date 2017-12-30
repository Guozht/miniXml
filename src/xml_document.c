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

#include "xml_element.h"

#include "xml_document.h"
#include "xml_document_struct.h"
#include "xml_reader.h"


XmlDocument * xml_document_new()
{
  XmlDocument * ret = (XmlDocument *) malloc(sizeof(XmlDocument));
  assert(ret);

  ret->encoding = strings_clone("UTF-8");
  ret->version = strings_clone("1.0");
  ret->root = xml_element_new("root");

  return ret;
}

XmlDocument * xml_document_new_with_root(XmlElement * root)
{
  assert(root);

  XmlDocument * ret = (XmlDocument *) malloc(sizeof(XmlDocument));
  assert(ret);

  ret->encoding = strings_clone("UTF-8");
  ret->version = strings_clone("1.0");
  ret->root = root;

  return ret;
}

XmlDocument * xml_document_parse(char * string, size_t string_length)
{
  assert(string);
  assert(string_length > 0);

  XmlReader * reader = xml_reader_new();
  XmlDocument * ret = xml_reader_parse_document(reader, string, string_length);
  xml_reader_destroy(reader);

  return ret;
}

void xml_document_destroy(XmlDocument * document)
{
  free(document->encoding);
  free(document->version);

  if (document->root)
    xml_element_destroy(document->root);
}


XmlElement * xml_document_get_root(XmlDocument * document)
{
  assert(document);

  return document->root;
}

char * xml_document_get_encoding(XmlDocument * document)
{
  assert(document);

  return strings_clone(document->encoding);
}

char * xml_document_get_version(XmlDocument * document)
{
  assert(document);

  return strings_clone(document->version);
}


void xml_document_set_encoding(XmlDocument * document, char * encoding)
{
  assert(document);
  assert(encoding);

  free(document->encoding);
  document->encoding = strings_clone(encoding);
}

void xml_document_set_version(XmlDocument * document, char * version)
{
  assert(document);
  assert(version);

  free(document->version);
  document->version = strings_clone(version);
}
