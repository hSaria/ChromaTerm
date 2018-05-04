/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct color_type color_table[] = {
    {"bold", "<188>"},         {"dim", "<288>"},
    {"underscore", "<488>"},   {"blink", "<588>"},
    {"azure", "<abd>"},        {"b azure", "<ABD>"},
    {"b black", "<880>"},      {"b blue", "<884>"},
    {"b cyan", "<886>"},       {"b ebony", "<G04>"},
    {"b green", "<882>"},      {"b jade", "<ADB>"},
    {"b lime", "<BDA>"},       {"b magenta", "<885>"},
    {"b orange", "<DBA>"},     {"b pink", "<DAB>"},
    {"b red", "<881>"},        {"b silver", "<CCC>"},
    {"b tan", "<CBA>"},        {"b violet", "<BAD>"},
    {"b white", "<887>"},      {"b yellow", "<883>"},
    {"black", "<808>"},        {"blue", "<848>"},
    {"cyan", "<868>"},         {"ebony", "<g04>"},
    {"green", "<828>"},        {"jade", "<adb>"},
    {"light azure", "<acf>"},  {"light ebony", "<bbb>"},
    {"light jade", "<afc>"},   {"light lime", "<cfa>"},
    {"light orange", "<fca>"}, {"light pink", "<fac>"},
    {"light silver", "<eee>"}, {"light tan", "<eda>"},
    {"light violet", "<caf>"}, {"lime", "<bda>"},
    {"magenta", "<858>"},      {"orange", "<dba>"},
    {"pink", "<dab>"},         {"red", "<818>"},
    {"silver", "<ccc>"},       {"tan", "<cba>"},
    {"violet", "<bad>"},       {"white", "<878>"},
    {"yellow", "<838>"},       {"", "<099>"}};

struct command_type command_table[] = {{"commands", do_commands},
                                       {"config", do_configure},
                                       {"exit", do_exit},
                                       {"help", do_help},
                                       {"highlight", do_highlight},
                                       {"quit", do_exit},
                                       {"read", do_read},
                                       {"run", do_run},
                                       {"unhighlight", do_unhighlight},
                                       {"write", do_write},
                                       {"", NULL}};

struct config_type config_table[] = {
    {"CHARSET", "The character set encoding used", config_charset},
    {"COMMAND CHAR", "The character used for commands", config_commandchar},
    {"CONVERT META", "Convert meta and control characters", config_convertmeta},
    {"HIGHLIGHT", "Highlight according to rules", config_highlight},
    {"", "", NULL}};

struct list_type list_table[LIST_MAX] = {{"CONFIG", ALPHA},
                                         {"HIGHLIGHT", PRIORITY}};
