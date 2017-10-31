

#include <assert.h>
#include <stdlib.h>
#include <baselib/baselib.h>

#include "xml_element.h"

#include "xml_document.h"
#include "xml_document_struct.h"


XmlDocument * xml_document_new()
{
  XmlDocument * ret = (XmlDocument *) malloc(sizeof(XmlDocument));
  assert(ret);

  ret->encoding = strings_clone("UTF-8");
  ret->version = strings_clone("1.0");
  ret->root = xml_element_new("root");

  return ret;
}

XmlDocument * xml_document_parse(char * string)
{
  assert(0); /* NOT YET IMPLEMENTED */
}

void xml_document_destroy(XmlDocument * document)
{
  free(document->encoding);
  free(document->version);
  if (document->root)
    xml_element_destroy(document->root);
}


XmlElement * xml_document_get_root(XmlDocument * document)
{
  assert(document);

  return document->root;
}

char * xml_document_get_encoding(XmlDocument * document)
{
  assert(document);

  return strings_clone(document->encoding);
}

char * xml_document_get_version(XmlDocument * document)
{
  assert(document);

  return strings_clone(document->version);
}


void xml_document_set_encoding(XmlDocument * document, char * encoding)
{
  assert(document);
  assert(encoding);

  document->encoding = strings_clone(encoding);
}

void xml_document_set_version(XmlDocument * document, char * version)
{
  assert(document);
  assert(version);

  document->version = strings_clone(version);
}





