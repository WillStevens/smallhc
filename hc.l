%option noyywrap
%option never-interactive

%{
#include <string>

using namespace std;

#include "hcast.h"
#include "hc.tab.h"

%}

NUMBER	([0-9]+)|(0x[0-9a-fA-F]+)
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*
WHITESPACE [ \t]

%%

void			{return VOID;}
main			{return MAIN;}
par				{return PAR;}
seq				{return SEQ;}
unsigned		{return UNSIGNED;}
signal			{return SIGNAL;}
ram				{return RAM;}
extram			{return EXTRAM;}
port			{return PORT;}
if				{return IF;}
else			{return ELSE;}
while			{return WHILE;}
delay			{return DELAY;}
{IDENTIFIER}	{yylval.identifier_t = new string(yytext); return IDENTIFIER;}
{NUMBER}		{yylval.number_t = new string(yytext); return NUMBER;}
[-+()&|^;=@{}:,]	{return yytext[0];}
"["|"]"			{return yytext[0];}
"=="			{return EQUAL_OP;}
"!="			{return UNEQUAL_OP;}
"||"			{return BOOL_OR;}
"&&"			{return BOOL_AND;}

{WHITESPACE}	;
"\n"			{LineNumber++;}

%%