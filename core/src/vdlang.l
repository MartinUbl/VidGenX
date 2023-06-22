%option noyywrap

%top {
    //#define DEBUG_PRINT(x) printf("SCANNER: " x "\n")
    #define DEBUG_PRINT(x)

    #include <string>
    #include "vdlang_parser.h"
}

ANYVALUE \'[0-9a-zA-Z\#\s\w]*\'
FLOATNUM [-]{0,1}[0-9]+[\.]{0,1}[0-9]*
INTNUM [-]{0,1}[0-9]+
STRING [a-zA-Z]+[a-zA-Z0-9_]*
TIMESPEC [-]{0,1}[0-9]+(s|m|ms){1}

%%

"Config"        { DEBUG_PRINT("config"); return IDENT_CONFIG; }
"Proto"         { DEBUG_PRINT("proto"); return IDENT_PROTO; }
"Constants"     { DEBUG_PRINT("constants"); return IDENT_CONSTANTS; }
"Scene"         { DEBUG_PRINT("scene"); return IDENT_SCENE; }
"{"             { DEBUG_PRINT("lbracket"); return L_BRACKET; }
"}"             { DEBUG_PRINT("rbracket"); return R_BRACKET; }
"("             { DEBUG_PRINT("lparen"); return L_PAREN; }
")"             { DEBUG_PRINT("rparen"); return R_PAREN; }
";"             { DEBUG_PRINT("semicolon"); return SEMICOLON; }
","             { DEBUG_PRINT("comma"); return COMMA; }
":"             { DEBUG_PRINT("colon"); return COLON; }
"="             { DEBUG_PRINT("equals"); return EQUALS; }
"->"            { DEBUG_PRINT("larrow"); return L_ARROW; }
"-"             { DEBUG_PRINT("dash"); return DASH; }
"RGB"           { DEBUG_PRINT("rgbspec"); return RGBSPEC; }
[ \t\r\n]       ;
"//".*          /* ignore comments */

{TIMESPEC} {
    DEBUG_PRINT("timespec (float)");

    yylval.floatval = atof(yytext);

    std::string tmp(yytext);
    if (tmp.ends_with("ms")) {
        // no need to convert
    }
    else if (tmp.ends_with("s")) {
        yylval.floatval *= 1000;
    }
    else if (tmp.ends_with("m")) {
        yylval.floatval *= 60*1000;
    }

    return TIMESPEC;
}

{FLOATNUM} {
    DEBUG_PRINT("number (float)");
    yylval.floatval = atof(yytext);
    return FLOAT_NUMBER;
}

{STRING} {
    DEBUG_PRINT("identifier");
    yylval.strval = (char*)malloc(strlen(yytext) + 1);
    memcpy(yylval.strval, yytext, strlen(yytext)+1);
    return IDENTIFIER;
}

{ANYVALUE} {
    DEBUG_PRINT("strvalue");
    yylval.strval = (char*)malloc(strlen(yytext) + 1 - 2);
    memcpy(yylval.strval, yytext + 1, strlen(yytext)+1 - 2);
    yylval.strval[strlen(yytext)-2] = '\0';
    return STRVALUE;
}

. ;
