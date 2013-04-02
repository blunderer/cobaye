
#ifndef MAIN_H
#define MAIN_H

#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "cobaye_menu_core.h"
#include "cobaye_display.h"
#include "cobaye_tests.h"
#include "cobaye_seq.h"

extern WINDOW *teststatuswin;
extern WINDOW *statuswin;
extern WINDOW *titlewin;
extern WINDOW *menuwin;
extern WINDOW *mainwin;
extern WINDOW *helpwin;
extern WINDOW *ctxwin;

int cobaye_ncurses_enabled(void);

#endif /* MAIN_H */
