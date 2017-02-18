/*
 * CS352 Spring 2015
 * Interpreter for miniscript
 * Andrew F. Davis
 */

#ifndef _MINISCRIPT_H
#define _MINISCRIPT_H

//#define LDEBUG
//#define RDEBUG

#ifdef LDEBUG
#   define ldprintf(...)       printf(__VA_ARGS__)
#else
#   define ldprintf(...)       do {} while(0)
#endif

#ifdef RDEBUG
#   define rdprintf(...)       printf(__VA_ARGS__)
#else
#   define rdprintf(...)       do {} while(0)
#endif

#endif // _MINISCRIPT_H
