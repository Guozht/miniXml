#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *#
#*                                                                         *#
#*  miniXml: a library implementing several simple utilities for C         *#
#*  Copyright (C) 2018  LeqxLeqx                                           *#
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

bin_dir=./bin
src_dir=./src
file_names=$(notdir $(wildcard $(src_dir)/*.c))
src_files=$(addprefix $(src_dir)/,$(file_names))
object_files=$(addprefix $(bin_dir)/,$(file_names:.c=.o))
CFLAGS=-std=gnu11 -Wall -g -lbaselib -lpthread -fPIC
DEBUG_FLAGS=
CC=gcc

.PHONY : all
all : $(object_files)
	$(CC) $(CFLAGS) -shared $^ -o $(bin_dir)/libminixml.so

debug : DEBUG_FLAGS=-DMTEST_DEBUG
debug : all


$(bin_dir)/%.o : $(src_dir)/%.c bin
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

bin :
	mkdir bin

.PHONY: clean
clean :
	rm -rf bin
