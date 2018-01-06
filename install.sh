#!/bin/bash
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *#
#*                                                                         *#
#*  miniXml: a simple XML parsing library for C                            *#
#*  Copyright (C) 2017  LeqxLeqx                                           *#
#*                                                                         *#
#*  This program is free software: you can redistribute it and/or modify   *#
#*  it under the terms of the GNU General Public License as published by   *#
#*  the Free Software Foundation, either version 3 of the License, or      *#
#*  (at your option) any later version.                                    *#
#*                                                                         *#
#*  This program is distributed in the hope that it will be useful,        *#
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of         *#
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *#
#*  GNU General Public License for more details.                           *#
#*                                                                         *#
#*  You should have received a copy of the GNU General Public License      *#
#*  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *#
#*                                                                         *#
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *#

LIBRARY_FILE_NAME=libminixml.so
HEADER_DIRECTORY_NAME=mini_xml

MAN_DIRECTORY_PATH=/usr/share/man/man3

if [[ $EUID != 0 ]] ; then
  echo "Must run as root"
  exit 8
fi

if [[ !(-e bin/$LIBRARY_FILE_NAME) ]] ; then
  echo "No binary file found. Run 'build.sh' before installing"
  exit 4
fi

if [[ !(-d /usr/include/$HEADER_DIRECTORY_NAME) ]] ; then
  mkdir /usr/include/$HEADER_DIRECTORY_NAME
else
  rm -f /usr/include/$HEADER_DIRECTORY_NAME/*
fi

cp "bin/$LIBRARY_FILE_NAME" "/usr/lib/$LIBRARY_PATH_NAME"

for f in `ls src/*.h`
do
  sudo cp $f /usr/include/$HEADER_DIRECTORY_NAME/
done

