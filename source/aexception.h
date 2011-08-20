/* See Copyright Notice in azure.h. */

#ifndef _AEXCEPTION_H_
#define _AEXCEPTION_H_

void throwit(avm* vm, word* w);
void throwdbz(avm* vm);
void throwfnf(avm* vm);
void throwcnlm(avm* vm);
void throwcnff(avm* vm);
void throwoor(avm* vm);
void throwia(avm* vm);

#endif
