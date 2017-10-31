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

  char * string = strings_from_array(raw_data, (unsigned int) raw_data_length);

  XmlReader * reader = xml_reader_new();

  printf("parsing %s... ", FILE_NAME);
  XmlElement * element = xml_reader_parse_element(reader, string);
  printf("done\n");
  
  if (!element)
  {
    char * error_message = xml_reader_get_error_message(reader);
    printf("Error Message: %s\n", error_message);
    free(error_message);
    
    return 1;
  }
  else
  {
    
    printf("parsed element: \n\n");
    
    XmlWriter * writer = xml_writer_new();
    xml_writer_set_style(writer, XML_WRITER_STYLE_COMPRESSED);
    xml_writer_write_element(writer, element, stdout);
    
    printf("\n\n=== end of element ===\n");
    
    return 0;
  }
}
