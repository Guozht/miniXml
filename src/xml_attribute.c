
#include <assert.h>
#include <baselib/baselib.h>
#include <stdlib.h>

#include "xml_attribute.h"
#include "xml_attribute_struct.h"


XmlAttribute * xml_attribute_new(char * name, char * value)
{
  assert(name);
  assert(value);

  XmlAttribute * ret = (XmlAttribute *) malloc(sizeof(XmlAttribute));
  assert(ret);

  ret->name = strings_clone(name);
  ret->value = strings_clone(value);

  return ret;
}

void xml_attribute_destroy(XmlAttribute * attribute)
{
  assert(attribute);

  free(attribute->name);
  free(attribute->value);

  free(attribute);
}

char * xml_attribute_get_name(XmlAttribute * attribute)
{
  assert(attribute);

  return strings_clone(attribute->name);
}

char * xml_attribute_get_value(XmlAttribute * attribute)
{
  assert(attribute);

  return strings_clone(attribute->value);
}
