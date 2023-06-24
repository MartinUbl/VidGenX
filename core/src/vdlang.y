%code requires {
   class CBlock;
   class CParams;
   class CCommand;
   class CAttributes;
   struct TParam_Entry;
   struct TValue_Spec;
   enum class NBlock_Type;
}

%code top {
    #include <iostream>
    #include <string>
    #include <list>
    #include <iostream>
    #include <string>
    #include <sstream>
    #include <iomanip>
    #include <bit>
    #include "parser_entities.h"

    extern int yylex(void);
    extern char* yytext;

    static void yyerror(const char* s) {
        std::cerr << s << std::endl;
    }

    rgb_t hexColorToARGB(const std::string& hexColor) {
        std::string color = hexColor;
        if (color[0] == '#')
            color.erase(0, 1);

        std::stringstream ss;
        ss << std::hex << color;
        rgb_t argb;
        ss >> argb;

        argb |= 0xFF000000;

        return std::bit_cast<rgb_t>(argb);
    }

    std::vector<CBlock*> _Blocks;
    size_t _Block_Counter = 0;
}

%union {
    int intval;
    double floatval;
    char *strval;
    CBlock *block;
    NBlock_Type block_type;
    CParams* params;
    TParam_Entry* param_entry;
    TValue_Spec* val_spec;
    CCommand* cmd;
    CAttributes* attrs;
}

%define parse.error verbose
%verbose

%token IDENT_CONFIG IDENT_PROTO IDENT_CONSTANTS IDENT_SCENE L_BRACKET R_BRACKET SEMICOLON L_ARROW COMMA L_PAREN R_PAREN DASH EQUALS COLON DOT
%token<floatval> INT_NUMBER FLOAT_NUMBER
%token<intval> RGBSPEC TIMESPEC
%token<strval> STRVALUE IDENTIFIER
%type<block> top_level_block
%type<block_type> top_level_identifier
%type<params> top_level_params
%type<params> params_block
%type<param_entry> param_spec
%type<val_spec> value_spec
%type<cmd> top_level_body command_block command
%type<attrs> attr_block attr_spec

%%

start
    : top_level_block_chain {
    }
;

top_level_block_chain
    : top_level_block {
        $1->Set_Block_Index(_Block_Counter++);
        _Blocks.push_back($1);
    }
    | top_level_block top_level_block_chain {
        $1->Set_Block_Index(_Block_Counter++);
        _Blocks.push_back($1);
    }
;

top_level_block
    : top_level_identifier top_level_params top_level_body {
        $$ = new CBlock($1);
        $$->Set_Params($2);
        $$->Set_Command($3);
    }
;

top_level_identifier
    : IDENT_CONFIG {
        $$ = NBlock_Type::Config;
    }
    | IDENT_PROTO {
        $$ = NBlock_Type::Prototypes;
    }
    | IDENT_CONSTANTS {
        $$ = NBlock_Type::Consts;
    }
    | IDENT_SCENE {
        $$ = NBlock_Type::Scene;
    }
;

top_level_params
    : L_PAREN params_block R_PAREN {
        $$ = $2;
    }
    | L_PAREN R_PAREN {
        $$ = new CParams();
    }
    | {
        $$ = new CParams();
    }
;

params_block
    : param_spec {
        $$ = new CParams();
        $$->Add_Parameter($1);
        delete $1;
    }
    | param_spec COMMA params_block {
        $3->Add_Parameter($1);
        delete $1;
        $$ = $3;
    }
;

param_spec
    : IDENTIFIER EQUALS value_spec {
        $$ = new TParam_Entry();
        $$->key = $1;
        $$->value = *$3;
        delete $3;
    }
;

value_spec
    : INT_NUMBER {
        $$ = new TValue_Spec{ NValue_Type::Float, $1 };
    }
    | FLOAT_NUMBER {
        $$ = new TValue_Spec{ NValue_Type::Float, $1 };
    }
    | STRVALUE {
        $$ = new TValue_Spec{ NValue_Type::String, $1 };
    }
    | IDENTIFIER {
        $$ = new TValue_Spec{ NValue_Type::Identifier, $1 };
    }
    | RGBSPEC L_PAREN STRVALUE R_PAREN {
        $$ = new TValue_Spec{ NValue_Type::RGB, hexColorToARGB($3) };
    }
    | TIMESPEC {
        $$ = new TValue_Spec{ NValue_Type::Timespec, $1 };
    }
;

top_level_body
    : L_BRACKET R_BRACKET {
        $$ = new CCommand();
    }
    | L_BRACKET command_block R_BRACKET {
        $$ = $2;
    }
    | {
        $$ = new CCommand();
    }
;

command_block
    : command {
        $$ = new CCommand();
        $$->Add_Command($1);
    }
    | command command_block {
        $$ = $2;
        $$->Add_Command($1);
    }
;

attr_block
    : attr_spec {
        $$ = $1;
    }
    | attr_spec COMMA attr_block {
        $$ = $3;
        $1->Merge_To($$);
        delete $1;
    }
;

attr_spec
    : IDENTIFIER {
        $$ = new CAttributes();
        $$->Add_Attribute($1);
    }
;

command
    : IDENTIFIER EQUALS IDENTIFIER L_PAREN params_block R_PAREN {
        $$ = new CCommand();
        $$->Set_Identifier($1);
        $$->Set_Entity_Name($3);
        $$->Set_Params($5);
    }
    | IDENTIFIER L_PAREN params_block R_PAREN {
        $$ = new CCommand();
        $$->Set_Entity_Name($1);
        $$->Set_Params($3);
    }
    | IDENTIFIER L_PAREN R_PAREN {
        $$ = new CCommand();
        $$->Set_Entity_Name($1);
    }
    | IDENTIFIER EQUALS value_spec {
        $$ = new CCommand();
        $$->Set_Identifier($1);
        $$->Set_Value(*$3);
        delete $3;
    }
    | IDENTIFIER EQUALS IDENTIFIER L_PAREN attr_block R_PAREN L_BRACKET command_block R_BRACKET {
        $$ = new CCommand();
        $$->Set_Identifier($1);
        $$->Set_Entity_Name($3);
        $$->Set_Attributes($5);
        $$->Add_Command($8);
    }
    | IDENTIFIER DOT IDENTIFIER L_PAREN params_block R_PAREN {
        $$ = new CCommand();
        $$->Set_Object_Reference($1);
        $$->Set_Entity_Name($3);
        $$->Set_Params($5);
    }
;
