/*
 * CS352 Spring 2015
 * Lexical analyzer for miniscript interpreter
 * Andrew F. Davis
 */

D  [0-9]
L  [a-zA-Z]
WS [ \t]

%{
#include <string>
#include "miniscript.hh"
#include "ast.hh"
#include "parser.hpp"

/* handle locations */
#define YY_USER_ACTION yylloc.first_line = yylineno;
%}

%option yylineno

%%

"<script type=\"text/JavaScript\">"     { ldprintf("START_TAG\n"); return START_TAG; }
"</script>"                             { ldprintf("STOP_TAG\n"); return STOP_TAG; }

"document.write"                        { ldprintf("DOC_WRITE\n"); return DOC_WRITE; }
"var"                                   { ldprintf("VAR\n"); return VAR; }
"if"                                    { ldprintf("IF\n"); return IF; }
"else"                                  { ldprintf("ELSE\n"); return ELSE; }
"while"                                 { ldprintf("WHILE\n"); return WHILE; }
"do"                                    { ldprintf("DO\n"); return DO; }
"break"                                 { ldprintf("BREAK\n"); return BREAK; }
"continue"                              { ldprintf("CONTINUE\n"); return CONTINUE; }
"true"                                  { ldprintf("TRUE\n"); return TRUE; }
"false"                                 { ldprintf("FALSE\n"); return FALSE; }
"function"                              { ldprintf("FUNCTION\n"); return FUNCTION; }
"return"                                { ldprintf("RETURN\n"); return RETURN; }
"assert"                                { ldprintf("ASSERT\n"); return ASSERT; }

{D}+                                    { yylval.int_val = atoi(yytext);
                                          ldprintf("INTEGER: %d\n", yylval.int_val);
                                          return INTEGER; }

{L}({L}|{D}|"_")*                       { yylval.string_val = strdup( yytext );
                                          ldprintf("ID: %s\n", yylval.string_val);
                                          return ID; }

"\"<br />\""                             { ldprintf("BRTAG\n"); return BRTAG; }

\"[^\"\n]*\"                            { yylval.string_val = strdup( yytext + 1 );
                                          yylval.string_val[strlen(yylval.string_val) - 1] = '\0';
                                          ldprintf("STRING: \"%s\"\n", yylval.string_val);
                                          return STRING; }

"="                                     { ldprintf("=\n"); return '='; }
"-"                                     { ldprintf("-\n"); return '-'; }
"+"                                     { ldprintf("+\n"); return '+'; }
"*"                                     { ldprintf("*\n"); return '*'; }
"/"                                     { ldprintf("/\n"); return '/'; }
">"                                     { ldprintf("GT\n"); return GT; }
"<"                                     { ldprintf("LT\n"); return LT; }
">="                                    { ldprintf("GE\n"); return GE; }
"<="                                    { ldprintf("LE\n"); return LE; }
"!="                                    { ldprintf("NE\n"); return NE; }
"=="                                    { ldprintf("EQ\n"); return EQ; }
"||"                                    { ldprintf("OR\n"); return OR; }
"&&"                                    { ldprintf("AND\n"); return AND; }
"."                                     { ldprintf(".\n"); return '.'; }
","                                     { ldprintf(",\n"); return ','; }
"("                                     { ldprintf("(\n"); return '('; }
")"                                     { ldprintf(")\n"); return ')'; }
"{"                                     { ldprintf("{\n"); return '{'; }
"}"                                     { ldprintf("}\n"); return '}'; }
"["                                     { ldprintf("[\n"); return '['; }
"]"                                     { ldprintf("]\n"); return ']'; }
":"                                     { ldprintf("COLON\n"); return COLON; }
";"                                     { ldprintf("SEMICOLON\n"); return SEMICOLON; }
"!"                                     { ldprintf("NOT\n"); return NOT; }

\n                                      { ldprintf("NEWLINE\n"); return NEWLINE; }

{WS}+                                   { /* Discard whitespace */ }

.                                       { /* Everything else is invalid */ }

%%

int yywrap(void)
{
	return 1;
}
