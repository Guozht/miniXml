

#ifndef __XML_DOCUMENT_H
#define __XML_DOCUMENT_H


struct XmlDocument;
typedef struct XmlDocument XmlDocument;


#include "xml_element.h"


XmlDocument * xml_document_new();
XmlDocument * xml_document_parse(char * string);

void xml_document_destroy(XmlDocument * document);


XmlElement * xml_document_get_root(XmlDocument * document);
char * xml_document_get_encoding(XmlDocument * document);
char * xml_document_get_version(XmlDocument * document);

void xml_document_set_encoding(XmlDocument * document, char * encoding);
void xml_document_set_version(XmlDocument * document, char * version);


#endif





