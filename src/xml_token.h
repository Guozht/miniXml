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

 
#ifndef __XML_TOKEN_H
#define __XML_TOKEN_H

enum XmlTokenType
{
  XML_TOKEN_TYPE_START_TAG,
  XML_TOKEN_TYPE_END_TAG,
  XML_TOKEN_TYPE_END_EMPTY_TAG,
  XML_TOKEN_TYPE_START_END_TAG,
  XML_TOKEN_TYPE_IDENTIFIER,
  XML_TOKEN_TYPE_EQUALS,
  XML_TOKEN_TYPE_ENTITY,
  XML_TOKEN_TYPE_QUOTED_STRING,
  XML_TOKEN_TYPE_TEXT,

  XML_TOKEN_TYPE_START_COMMENT_TAG,
  XML_TOKEN_TYPE_END_COMMENT_TAG,

  XML_TOKEN_TYPE_END_OF_FILE,
};

struct XmlToken
{
  enum XmlTokenType type;
  char * data;
};
typedef struct XmlToken XmlToken;




XmlToken * xml_token_new(enum XmlTokenType type, char * data);
void xml_token_destroy(XmlToken * token);


char * xml_token_to_string(XmlToken * type);


#endif
