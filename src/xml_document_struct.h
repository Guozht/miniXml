
#ifndef __XML_DOCUMENT_STRUCT_H
#define __XML_DOCUMENT_STRUCT_H


#include "xml_element.h"


struct XmlDocument
{
  char * encoding;
  char * version;
  XmlElement * root;
};

#endif