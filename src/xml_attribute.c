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
#include <stdlib.h>

#include "xml_attribute.h"
#include "xml_attribute_struct.h"


XmlAttribute * xml_attribute_new(char * name, char * value)
{
  assert(name);
  assert(value);

  XmlAttribute * ret = (XmlAttribute *) malloc(sizeof(XmlAttribute));
  assert(ret);

  ret->name = strings_clone(name);
  ret->value = strings_clone(value);

  return ret;
}

void xml_attribute_destroy(XmlAttribute * attribute)
{
  assert(attribute);

  free(attribute->name);
  free(attribute->value);

  free(attribute);
}

char * xml_attribute_get_name(XmlAttribute * attribute)
{
  assert(attribute);

  return strings_clone(attribute->name);
}

char * xml_attribute_get_value(XmlAttribute * attribute)
{
  assert(attribute);

  return strings_clone(attribute->value);
}
