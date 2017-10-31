

#ifndef __XML_TOKENIZER_H
#define __XML_TOKENIZER_H


#include <baselib/baselib.h>


struct XmlTokenizer
{
  char 
    * string,
    * error_message;
  unsigned int 
    string_length, 
    current,
    line,
    column;
  LinkedList * tokens;
};
typedef struct XmlTokenizer XmlTokenizer;

XmlTokenizer * xml_tokenizer_new(char * string);
void xml_tokenizer_destroy(XmlTokenizer * tokenizer);


char xml_tokenizer_character(XmlTokenizer * tokenizer);
LinkedList * xml_tokenizer_tokenize(XmlTokenizer * tokenizer);


#endif
