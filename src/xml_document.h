

#ifndef __XML_DOCUMENT_H
#define __XML_DOCUMENT_H


struct XmlDocument;
typedef struct XmlDocument XmlDocument;


#include "xml_element.h"


XmlDocument * xml_document_new();
XmlDocument * xml_document_new_with_root(XmlElement * root);
XmlDocument * xml_document_parse(char * string, size_t string_length);

void xml_document_destroy(XmlDocument * document);


XmlElement * xml_document_get_root(XmlDocument * document);
char * xml_document_get_encoding(XmlDocument * document);
char * xml_document_get_version(XmlDocument * document);

void xml_document_set_encoding(XmlDocument * document, char * encoding);
void xml_document_set_version(XmlDocument * document, char * version);


#endif
