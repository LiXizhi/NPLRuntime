#include "OpenGL.h"
#import <mach-o/dyld.h>
#import <stdlib.h>
#import <string.h>
void *NSGLGetProcAddress(const char *name)
{
    NSSymbol symbol;
    char *symbolName;
    // Prepend a '_' for the Unix C symbol mangling convention
    symbolName = malloc (strlen (name) + 2);
    strcpy(symbolName + 1, name);
    symbolName[0] = '_';
    symbol = NULL;
    if (NSIsSymbolNameDefined (symbolName))
        symbol = NSLookupAndBindSymbol (symbolName);
    free (symbolName);
    return symbol ? NSAddressOfSymbol (symbol) : NULL;
}
int loadGL()
{
	return gladLoadGLLoader(NSGLGetProcAddress);
}
