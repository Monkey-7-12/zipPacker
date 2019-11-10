
/*  dirwalk.h
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


#ifndef DIRWALK_H
#define DIRWALK_H

extern void dir_walker(char* name, void (*f_callback)(), void (*d_callback)());

#endif /* DIRWALK_H */

