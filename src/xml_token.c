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

 

#include "xml_token.h"

#include <assert.h>
#include <baselib/baselib.h>
#include <stdlib.h>

XmlToken * xml_token_new(enum XmlTokenType type, char * data)
{
  XmlToken * ret = (XmlToken *) malloc(sizeof(XmlToken));
  assert(ret);

  ret->type = type;
  ret->data = data != NULL ? strings_clone(data) : NULL;

  return ret;
}

void xml_token_destroy(XmlToken * token)
{
  assert(token);

  if (token->data)
    free(token->data);

  free(token);
}


char * xml_token_to_string(XmlToken * token)
{
  switch (token->type)
  {

    case XML_TOKEN_TYPE_START_TAG:
      return strings_clone("<");
    case XML_TOKEN_TYPE_END_TAG:
      return strings_clone(">");
    case XML_TOKEN_TYPE_END_EMPTY_TAG:
      return strings_clone("/>");
    case XML_TOKEN_TYPE_START_END_TAG:
      return strings_clone("</");
    case XML_TOKEN_TYPE_IDENTIFIER:
      return strings_clone(token->data);
    case XML_TOKEN_TYPE_EQUALS:
      return strings_clone("=");
    case XML_TOKEN_TYPE_ENTITY:
      return strings_format("&%s;", token->data);
    case XML_TOKEN_TYPE_QUOTED_STRING:
      return strings_format("\"%s\"", token->data);
    case XML_TOKEN_TYPE_TEXT:
      return strings_clone(token->data);
    case XML_TOKEN_TYPE_START_COMMENT_TAG:
      return strings_clone("<!--");
    case XML_TOKEN_TYPE_END_COMMENT_TAG:
      return strings_clone("-->");

    default:
      assert(0);
  }
}
