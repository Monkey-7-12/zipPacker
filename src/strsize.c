
/*  strsize.c
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
   
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h> /* pow(), round() */


double _round(double x, int precision) {
    if (precision > 0) {
        double _precision = pow(10, precision);
        return roundl(x*_precision)/_precision;
    }
	return round(x);
}

char *size2str(uint64_t bytes, int precision)
{
	static char __s_strsize[20+1];

	char suffixes[] = {'b', 'k', 'M', 'G', 'T'};
	char sfx_count = sizeof(suffixes) / sizeof(suffixes[0]);


	double _bytes = bytes;
    int i = 0;

	while (_bytes > 1024 && i < sfx_count-1) {
		i++;
		_bytes /= 1024.0;
	}

    // FIXME _fmt... ...
	char _fmt[25];
	sprintf(_fmt, "%%.%df%%c", precision);
	sprintf(__s_strsize, _fmt, _round(_bytes, precision), suffixes[i]);
	return __s_strsize;
}


// FIXME / Rewrite
uint64_t str2size(char *s) {
	int len = strlen(s);
	char c = tolower(s[len-1]);
	long m = 0;
	char stmp[len+1];
	strcpy(stmp, s);

	switch (c) {
		case 'b': m = 1; break;
		case 'k': m = 1024; break;
		case 'm': m = 1048576; break;
		case 'g': m = 1073741824; break;
		case 't': m = 1099511627776; break;
		default:
			if (isdigit(c)) m = 1;
			else return -1;
			break;
	}
	return strtoll(s, NULL, 0) * m;
}

/*
vim: ts=4:sw=4:
*/
