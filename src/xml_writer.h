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

 
#ifndef __XML_WRITER_H
#define __XML_WRITER_H

struct XmlWriter;
typedef struct XmlWriter XmlWriter;


enum XmlWriterStyle
{
  XML_WRITER_STYLE_STANDARD,
  XML_WRITER_STYLE_COMPRESSED,
  XML_WRITER_STYLE_EXPANDED,
};


#include <assert.h>
#include <baselib/baselib.h>
#include <stdio.h>
#include <stdlib.h>

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_element.h"



XmlWriter * xml_writer_new();
void xml_writer_destroy(XmlWriter * writer);


enum XmlWriterStyle xml_writer_get_style(XmlWriter * writer);
unsigned int xml_writer_get_indent_width(XmlWriter * writer);

void xml_writer_set_style(XmlWriter * writer, enum XmlWriterStyle style);
void xml_writer_set_indent_width(XmlWriter * writer, unsigned int width);


char * xml_writer_get_document_text(XmlWriter * writer, XmlDocument * document);
char * xml_writer_get_element_text(XmlWriter * writer, XmlElement * element);

void xml_writer_print_document(XmlWriter * writer, XmlDocument * document, char * string);
void xml_writer_printn_document(
    XmlWriter * writer, XmlDocument * document,
    char * string, unsigned int n);

void xml_writer_print_element(XmlWriter * writer, XmlElement * element, char * string);
void xml_writer_printn_element(
    XmlWriter * writer, XmlElement * element,
    char * string, unsigned int n);

void xml_writer_write_document(XmlWriter * writer, XmlDocument * document, FILE * stream);
void xml_writer_write_element(XmlWriter * writer, XmlElement * element, FILE * stream);


#endif
