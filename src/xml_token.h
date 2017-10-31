
#ifndef __XML_TOKEN_H
#define __XML_TOKEN_H

enum XmlTokenType
{
  XML_TOKEN_TYPE_START_TAG,
  XML_TOKEN_TYPE_END_TAG,
  XML_TOKEN_TYPE_END_EMPTY_TAG,
  XML_TOKEN_TYPE_START_END_TAG,
  XML_TOKEN_TYPE_IDENTIFIER,
  XML_TOKEN_TYPE_EQUALS,
  XML_TOKEN_TYPE_ENTITY,
  XML_TOKEN_TYPE_QUOTED_STRING,
  XML_TOKEN_TYPE_TEXT,
  
  XML_TOKEN_TYPE_START_COMMENT_TAG,
  XML_TOKEN_TYPE_END_COMMENT_TAG,
  
  XML_TOKEN_TYPE_END_OF_FILE,
};

struct XmlToken
{
  enum XmlTokenType type;
  char * data;
};
typedef struct XmlToken XmlToken;




XmlToken * xml_token_new(enum XmlTokenType type, char * data);
void xml_token_destroy(XmlToken * token);


/* <REMOVE> */
char * xml_token_type_get_string(enum XmlTokenType type);
/* </REMOVE> */


#endif



