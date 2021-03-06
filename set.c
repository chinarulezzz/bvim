/* Bvim - BVi IMproved, binary analysis framework
 *
 * Copyright 1996-2004 by Gerhard Buergmann <gerhard@puon.at>
 * Copyright 2011 by Anton Kochkov <anton.kochkov@gmail.com>
 *
 * This file is part of Bvim.
 *
 * Bvim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bvim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bvim.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include "bvim.h"
#include "blocks.h"
#include "set.h"
#include "ui.h"
#include "keys.h"

//extern core_t core;

int from_file;
static FILE *ffp;
static char buf[64];

struct param params[] = {
	{"autowrite", "aw", FALSE, "", P_BOOL},
	{"columns", "cm", 16, "", P_NUM},
	{"errorbells", "eb", FALSE, "", P_BOOL},
	{"ignorecase", "ic", FALSE, "", P_BOOL},
	{"magic", "ma", TRUE, "", P_BOOL},
	{"memmove", "mm", FALSE, "", P_BOOL},
	{"offset", "of", 0, "", P_NUM},
	{"readonly", "ro", FALSE, "", P_BOOL},
	{"scroll", "scroll", 12, "", P_NUM},
	{"showmode", "mo", TRUE, "", P_BOOL},
	{"term", "term", 0, "", P_TEXT},
	{"terse", "terse", FALSE, "", P_BOOL},
	{"unixstyle", "us", FALSE, "", P_BOOL},
	{"window", "window", 25, "", P_NUM},
	{"wordlength", "wl", 4, "", P_NUM},
	{"wrapscan", "ws", TRUE, "", P_BOOL},
	{"", "", 0, "", 0,}	/* end marker */
};

int doset(core_t *core, char *arg)
{
	int i;
	char *s;
	int did_window = FALSE;
	int state = TRUE;	/* new state of boolean parms. */
	char string[80];

	if (arg == NULL) {
		showparms(core, FALSE);
		return 0;
	}
	if (!strcmp(arg, "all")) {
		showparms(core, TRUE);
		return 0;
	}
	if (!strncmp(arg, "no", 2)) {
		state = FALSE;
		arg += 2;
	}

	/* extract colors section */
	if (!strncmp(arg, "color", 5)) {
		arg = bvim_substr(arg, 6, -1);
		ui__Color_Set(core, core->curbuf, arg);
		return 0;
	} else {
		bvim_error(core, core->curbuf, arg);
		return 1;
	}

	for (i = 0; params[i].fullname[0] != '\0'; i++) {
		s = params[i].fullname;
		if (strncmp(arg, s, strlen(s)) == 0)	/* matched full name */
			break;
		s = params[i].shortname;
		if (strncmp(arg, s, strlen(s)) == 0)	/* matched short name */
			break;
	}

	if (params[i].fullname[0] != '\0') {	/* found a match */
		if (arg[strlen(s)] == '?') {
			if (params[i].flags & P_BOOL)
				sprintf(buf, "    %s%s",
					(params[i].nvalue ? "  " : "no"),
					params[i].fullname);
			else if (params[i].flags & P_TEXT)
				sprintf(buf, "      %s=%s", params[i].fullname,
					params[i].svalue);
			else
				sprintf(buf, "      %s=%ld", params[i].fullname,
					params[i].nvalue);
			bvim_info(core, core->curbuf, buf);
			return 0;
		}
		if (!strcmp(params[i].fullname, "term")) {
			bvim_error(core, core->curbuf, "Can't change type of terminal from within bvim");
			return 1;
		}
		if (params[i].flags & P_NUM) {
			if ((i == P_LI) || (i == P_OF))
				did_window++;
			if (arg[strlen(s)] != '=' || state == FALSE) {
				sprintf(string, "Option %s is not a toggle", params[i].fullname);
				bvim_error(core, core->curbuf, string);
				return 1;
			} else {
				s = arg + strlen(s) + 1;
				if (*s == '0') {
					params[i].nvalue = strtol(s, &s, 16);
				} else {
					params[i].nvalue = strtol(s, &s, 10);
				}
				params[i].flags |= P_CHANGED;

				if (i == P_CM) {
					if (((COLS - core->params.COLUMNS_ADDRESS - 1) / 4) >= P(P_CM)) {
						core->params.COLUMNS_DATA = P(P_CM);
					} else {
						core->params.COLUMNS_DATA = P(P_CM) = ((COLS - core->params.COLUMNS_ADDRESS - 1) / 4);
					}
					core->screen.maxx = core->params.COLUMNS_DATA * 4 + core->params.COLUMNS_ADDRESS + 1;
					core->params.COLUMNS_HEX = core->params.COLUMNS_DATA * 3;
					status = core->params.COLUMNS_HEX + core->params.COLUMNS_DATA - 17;
					screen = core->params.COLUMNS_DATA * (core->screen.maxy - 1);
					did_window++;
					stuffin("H");	/* set cursor at HOME */
				}
			}
		} else {	/* boolean */
			if (arg[strlen(s)] == '=') {
				bvim_error(core, core->curbuf, "Invalid set of boolean parameter");
				return 1;
			} else {
				params[i].nvalue = state;
				params[i].flags |= P_CHANGED;
			}
		}
	} else {
		bvim_error(core, core->curbuf, "No such option@- `set all' gives all option values");
		return 1;
	}

	if (did_window) {
		core->screen.maxy = P(P_LI) - 1;
		ui__Screen_New(core, core->curbuf);
	}

	return 0;
}

/* show ALL parameters */
void showparms(core_t *core, int all)
{
	struct param *p;
	int n;

	n = 2;
	bvim_info(core, core->curbuf, "Parameters:\n");
	for (p = &params[0]; p->fullname[0] != '\0'; p++) {
		if (!all && ((p->flags & P_CHANGED) == 0))
			continue;
		if (p->flags & P_BOOL)
			sprintf(buf, "    %s%s\n", (p->nvalue ? "  " : "no"), p->fullname);
		else if (p->flags & P_TEXT)
			sprintf(buf, "      %s=%s\n", p->fullname, p->svalue);
		else
			sprintf(buf, "      %s=%ld\n", p->fullname, p->nvalue);

		bvim_info(core, core->curbuf, buf);
		n++;
		if (n == params[P_LI].nvalue) {
			if (wait_return(core, core->curbuf, FALSE))
				return;
			n = 1;
		}
	}
	wait_return(core, core->curbuf, TRUE);
}

int getcmdstr(core_t *core, char* p, int x)
{
	int c;
	int n;
	char *buff, *q;

	attron(COLOR_PAIR(C_CM + 1));

	if (from_file) {
		if (fgets(p, 255, ffp) != NULL) {
			strtok(p, "\n\r");
			return 0;
		} else {
			return 1;
		}
	}

	signal(SIGINT, jmpproc);
	buff = p;
	move(core->screen.maxy, x);
	do {
		switch (c = vgetc()) {
		case BVI_CTRL('H'):
		case KEY_BACKSPACE:
		case KEY_LEFT:
			if (p > buff) {
				p--;
				move(core->screen.maxy, x);
				n = x;
				for (q = buff; q < p; q++) {
					addch(*q);
					n++;
				}
				addch(' ');
				move(core->screen.maxy, n);
			} else {
				*buff = '\0';
				bvim_info(core, core->curbuf, "");
				attroff(COLOR_PAIR(C_CM + 1));
				signal(SIGINT, SIG_IGN);
				return 1;
			}
			break;
		case ESC:	/* abandon command */
			*buff = '\0';
			bvim_info(core, core->curbuf, "");
			attroff(COLOR_PAIR(C_CM + 1));
			signal(SIGINT, SIG_IGN);
			return 1;
#if NL != KEY_ENTER
		case NL:
#endif
#if CR != KEY_ENTER
		case CR:
#endif
		case KEY_ENTER:
			break;
		default:	/* a normal character */
			addch(c);
			*p++ = c;
			break;
		}
		refresh();
	} while (c != NL && c != CR && c != KEY_ENTER);
	attroff(COLOR_PAIR(C_CM + 1));

	*p = '\0';
	signal(SIGINT, SIG_IGN);
	return 0;
}
