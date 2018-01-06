# miniXml

A simple and small XML parsing library for C

# DTD

At present, DTD is not supported and tags like `<!DOCTYPE ... >` will not be recognized as valid XML. However `<![CDATA[ .. ]]>` tags are supported

# Dependencies

This library relies upon `baselib`, which is available for download and installation at `https://github.com/LeqxLeqx/baselib`. `baselib`
itself relies upon the pthread libraries in the GNU C standard libraries.

# Compiling the Library

To compile and install this library, first run `./baselib.sh` and then `sudo ./install.sh`.

# Compiling Against the Library

To compile against this library, add `#include <mini_xml/mini_xml.h>` to the neccessary source code, and compile with the linkage 
flags `-lminixml -lbaselib -lpthread`.
