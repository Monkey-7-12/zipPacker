
/*  strsize.h
 *
 *  This file is part of zipPacker
 *
 *  Copyright © 1990−2019 by nýx.ch, CC-BY 4.0
 *
 *  Dieses Werk ist lizenziert unter der CC-BY
 *  Creative Commons Namensnennung 4.0 International Lizenz.
 *  https://creativecommons.org/licenses/by/4.0/deed.de
 *
 */

#ifndef STRSIZE_H
#define STRSIZE_H

extern char *size2str(uint64_t bytes, int precision);
extern uint64_t str2size(char *s);

#endif /* STRSIZE_H */

/*
vim: ts=4:sw=4:
*/
