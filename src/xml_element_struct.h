
#ifndef __XML_ELEMENT_STRUCT_H
#define __XML_ELEMENT_STRUCT_H

#include <baselib/baselib.h>

struct XmlElement
{
  char * name;
  ArrayList * attributes;
  ArrayList * children;
};


#endif
