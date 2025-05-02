#pragma once
#include "../GlobalVariables.h"
#include <vector>
unsigned int getIndexOfCount(int count, String input, String str, unsigned int fromIndex = 0);
void findSplitPosition(String text, int charIndex, int &posX, int &posY, int direction = -1);
int findCharIndex(String text, int &charIndex, int direction = -1);
String SplitString(String text);
unsigned int getIndexOfCount(int count, String input, String str, unsigned int fromIndex);
String HEXTOASCII(String hex);
int measureStringHeight(const String &text);