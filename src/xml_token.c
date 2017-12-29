

#include "xml_token.h"

#include <assert.h>
#include <baselib/baselib.h>
#include <stdlib.h>

XmlToken * xml_token_new(enum XmlTokenType type, char * data)
{
  XmlToken * ret = (XmlToken *) malloc(sizeof(XmlToken));
  assert(ret);

  ret->type = type;
  ret->data = data != NULL ? strings_clone(data) : NULL;

  return ret;
}

void xml_token_destroy(XmlToken * token)
{
  assert(token);

  if (token->data)
    free(token->data);

  free(token);
}

/*
 
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
  XML_TOKEN_TYPE_END_COMMENT_TAG
 */

/* TODO: remove method */
char * xml_token_type_get_string(enum XmlTokenType type)
{
  switch (type)
  {
    
    case XML_TOKEN_TYPE_START_TAG:
      return "<";
    case XML_TOKEN_TYPE_END_TAG:
      return ">";
    case XML_TOKEN_TYPE_END_EMPTY_TAG:
      return "/>";
    case XML_TOKEN_TYPE_START_END_TAG:
      return "</";
    case XML_TOKEN_TYPE_IDENTIFIER:
      return "IDENT";
    case XML_TOKEN_TYPE_EQUALS:
      return "=";
    case XML_TOKEN_TYPE_ENTITY:
      return "&ent;";
    case XML_TOKEN_TYPE_QUOTED_STRING:
      return "\"string\"";
    case XML_TOKEN_TYPE_TEXT:
      return "text";
    case XML_TOKEN_TYPE_START_COMMENT_TAG:
      return "<!--";
    case XML_TOKEN_TYPE_END_COMMENT_TAG:
      return "-->";
    
    default:
      return "UNKNOWN";
  }
}
