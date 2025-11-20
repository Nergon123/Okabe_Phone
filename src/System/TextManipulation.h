#pragma once
#include "../GlobalVariables.h"
#include <vector>
unsigned int getIndexOfCount(int count, NString input, NString str, unsigned int fromIndex = 0);
void findSplitPosition(NString text, size_t charIndex, int &posX, int &posY, int direction = -1);
int findCharPosX(NString text, int &charIndex, int direction = -1);
NString SplitString(NString text);
unsigned int getIndexOfCount(int count, NString input, NString str, unsigned int fromIndex);
NString HEXTOASCII(NString hex);
int measureStringHeight(const NString &text);