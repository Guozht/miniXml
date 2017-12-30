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

 

#ifndef __XML_DOCUMENT_H
#define __XML_DOCUMENT_H


struct XmlDocument;
typedef struct XmlDocument XmlDocument;


#include "xml_element.h"


XmlDocument * xml_document_new();
XmlDocument * xml_document_new_with_root(XmlElement * root);
XmlDocument * xml_document_parse(char * string, size_t string_length);

void xml_document_destroy(XmlDocument * document);


XmlElement * xml_document_get_root(XmlDocument * document);
char * xml_document_get_encoding(XmlDocument * document);
char * xml_document_get_version(XmlDocument * document);

void xml_document_set_encoding(XmlDocument * document, char * encoding);
void xml_document_set_version(XmlDocument * document, char * version);


#endif
