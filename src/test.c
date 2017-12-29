#include <baselib/baselib.h>
#include <stdio.h>
#include <stdlib.h>


#include "xml_document.h"
#include "xml_element.h"
#include "xml_attribute.h"
#include "xml_writer.h"
#include "xml_reader.h"

#define FILE_NAME "test.xml"

int main ( void )
{

  size_t raw_data_length;
  char * raw_data = files_read_all(FILE_NAME, &raw_data_length);

  XmlReader * reader = xml_reader_new();
  printf("parsing %s... ", FILE_NAME);
  XmlDocument * document = xml_reader_parse_document(
      reader,
      raw_data,
      raw_data_length
    );
  printf("done\n");

  if (!document)
  {
    printf("Parse Failed!\n");

    char * error_message = xml_reader_get_error_message(reader);
    printf("Error Message: %s\n", error_message);
    free(error_message);

    return 1;
  }
  else
  {

    printf("parsed document: \n\n");

    XmlWriter * writer = xml_writer_new();
    xml_writer_set_style(writer, XML_WRITER_STYLE_COMPRESSED);
    xml_writer_write_document(writer, document, stdout);

    printf("\n\n=== end of element ===\n");

    XmlElement * root = xml_document_get_root(document);
    XmlElement * sub = xml_element_get_child(root, "book");

    XmlAttribute * attrib = xml_element_get_attribute(sub, "id");
    char * value = xml_attribute_get_value(attrib);
    printf("value root->book-(attrib)->d: %s\n", value);
    free(value);

    return 0;
  }
}
