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



#ifndef __XML_TOKENIZER_H
#define __XML_TOKENIZER_H


#include <baselib/baselib.h>


struct XmlTokenizer
{
  char
    * string,
    * error_message;
  unsigned int
    string_length,
    current,
    line,
    column,
    last_columnt_length;
  LinkedList * tokens;
};
typedef struct XmlTokenizer XmlTokenizer;

XmlTokenizer * xml_tokenizer_new(char * string);
void xml_tokenizer_destroy(XmlTokenizer * tokenizer);


char xml_tokenizer_character(XmlTokenizer * tokenizer);
LinkedList * xml_tokenizer_tokenize(XmlTokenizer * tokenizer);
LinkedList * xml_tokenizer_tokenize_declaration(XmlTokenizer * tokenizer);

#endif
