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



#ifndef __XML_ELEMENT_H
#define __XML_ELEMENT_H


struct XmlElement;
typedef struct XmlElement XmlElement;


#include <baselib/baselib.h>
#include <stdbool.h>

#include "xml_attribute.h"


XmlElement * xml_element_new(char * name);
XmlElement * xml_element_parse(char * string);

void xml_element_destroy(XmlElement * element);



char * xml_element_get_name(XmlElement * element);
List * xml_element_get_attributes(XmlElement * element);
List * xml_element_get_children(XmlElement * element);
List * xml_element_get_child_elements(XmlElement * element);
bool xml_element_is_empty(XmlElement * element);

XmlAttribute * xml_element_get_attribute(XmlElement * element, char * name);

XmlElement * xml_element_get_child(XmlElement * element, char * name);
XmlElement * xml_element_get_child_ignore_case(XmlElement * element, char * name);
List * xml_element_get_children_by_name(XmlElement * element, char * name);
List * xml_element_get_children_by_name_ignore_case(XmlElement * element, char * name);

char * xml_element_get_value(XmlElement * element);

void xml_element_set_name(XmlElement * element, char * name);
void xml_element_set_value(XmlElement * element, char * value);

void xml_element_clear_attributes(XmlElement * element);
void xml_element_clear_children(XmlElement * element);

void xml_element_add_attribute(XmlElement * element, XmlAttribute * attribute);
void xml_element_add_child(XmlElement * element, XmlElement * child);
void xml_element_add_text(XmlElement * element, char * text);

void xml_element_add_attributes(XmlElement * element, List * attributes);
void xml_element_add_children(XmlElement * element, List * children);



#endif
