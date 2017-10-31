

#ifndef __XML_ATTRIBUTE_H
#define __XML_ATTRIBUTE_H


struct XmlAttribute;
typedef struct XmlAttribute XmlAttribute;


XmlAttribute * xml_attribute_new(char * name, char * value);
void xml_attribute_destroy(XmlAttribute * attribute);

char * xml_attribute_get_name(XmlAttribute * attribute);
char * xml_attribute_get_value(XmlAttribute * attribute);



#endif
