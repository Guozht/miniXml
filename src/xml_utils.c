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

#include <baselib/baselib.h>
#include <stdbool.h>


#include "xml_utils.h"

char * xml_utils_escape_string(char * str, bool attribute)
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
