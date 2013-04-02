
#ifndef DISPLAY_H
#define DISPLAY_H

#include "cobaye_ncurses.h"

void cobaye_display_help(char *txt);
void cobaye_display_status(char *txt, ...);
void cobaye_display_cobaye_status(int id, char *txt);
void cobaye_display_cobaye_vstatus(int id, char *txt, va_list list);

#endif /* DISPLAY_H */
