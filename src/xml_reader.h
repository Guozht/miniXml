
#ifndef __XML_READER_H
#define __XML_READER_H


#include "xml_document.h"
#include "xml_document_struct.h"
#include "xml_element.h"
#include "xml_element_struct.h"
#include "xml_attribute.h"
#include "xml_attribute_struct.h"


struct XmlReader;
typedef struct XmlReader XmlReader;


XmlReader * xml_reader_new();
void xml_reader_destroy(XmlReader * reader);

char * xml_reader_get_error_message(XmlReader * reader);

/* CAN BE ENCODED IN ANY MANNER WHICH IS ACCEPTABLE */
XmlDocument * xml_reader_parse_document(XmlReader * reader, char * data, size_t data_size);

/* EXPECTS UTF-8 */
XmlElement * xml_reader_parse_element(XmlReader * reader, char * data);




#endif
