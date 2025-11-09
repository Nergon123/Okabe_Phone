#include "TextManipulation.h"


// Function to get the index of the nth occurrence of a substring in a string
// @param count The occurrence number to find
// @param input The input string
// @param str The substring to find
// @param fromIndex The index to start searching from
// @return The index of the nth occurrence of the substring
unsigned int getIndexOfCount(int count, NString input, NString str, unsigned int fromIndex) {
    for (int i = 0; i < count; i++) { fromIndex = input.indexOf(str, fromIndex + 1); }
    return fromIndex;
}

// Function to find the split position of a string
// @param text The input string
// @param charIndex The character index to find the split position for
// @param posX The x-coordinate of the split position
// @param posY The y-coordinate of the split position
// @param direction The direction to search (UP or DOWN)
void findSplitPosition(NString text, int charIndex, int &posX, int &posY, int direction) {
    int lastNewLine  = 0;
    int curPosInText = 0;
    posX             = 0;
    posY             = 0;
    for (; curPosInText < charIndex; curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText + 1).c_str());
        if (posX >= 240 || text[curPosInText] == '\n') {
            posX        = 0;
            lastNewLine = curPosInText + 1;
            posY += tft.fontHeight();
        }
        if (curPosInText >= text.length() - 1) { break; }
    }
}

// Function to find the x position of a character in next or previous line based on the direction
// @param text The input string
// @param charIndex The character index to find the next x position for
// @param direction The direction to search (UP or DOWN)
// @return The new character index after moving in the specified direction
int findCharPosX(NString text, int &charIndex, int direction) {
    int prevNL       = 0;
    int lastNewLine  = 0;
    int nextNL       = text.length();
    int curPosInText = 0;
    int posX         = 0;
    int targetX      = 0;
    for (; curPosInText <= charIndex && curPosInText < text.length(); curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText).c_str());
        if (posX >= 240 || text[curPosInText] == '\n') {
            prevNL      = lastNewLine;
            lastNewLine = curPosInText + 1;
            posX        = 0;
        }
        if (curPosInText == charIndex) { targetX = posX; }
    }
    for (int i = lastNewLine; i < text.length(); i++) {
        if (text[i] == '\n' || tft.textWidth(text.substring(lastNewLine, i + 1)) >= 240) {
            nextNL = i;
            break;
        }
    }
    if (direction == UP && prevNL >= 0) {
        curPosInText = prevNL;
        for (; curPosInText < lastNewLine; curPosInText++) {
            posX = tft.textWidth(text.substring(prevNL, curPosInText + 1));
            if (posX >= targetX + 5 || text[curPosInText] == '\n') { break; }
        }
    }
    else if (direction == DOWN && nextNL < text.length()) {
        lastNewLine = nextNL + 1;
        for (int i = lastNewLine; i < text.length(); i++) {
            if (text[i] == '\n' || tft.textWidth(text.substring(lastNewLine, i + 1)) >= 240 ||
                i == text.length() - 1) {
                nextNL = i;
                break;
            }
        }
        curPosInText = lastNewLine;
        posX         = 0;
        for (; targetX >= posX; curPosInText++) {
            posX = tft.textWidth(text.substring(lastNewLine, curPosInText + 1));
            if (posX >= targetX || text[curPosInText] == '\n' || curPosInText == text.length()) {
                break;
            }
        }
    }
    charIndex = constrain(curPosInText, 0, text.length() - 1);
    return charIndex;
}

// Function to split a string into lines that fit within the screen width
// @param text The input string
// @return The formatted string with line breaks
NString SplitString(NString text) {
    NString result           = "";
    int    wordStart        = 0;
    int    wordEnd          = 0;
    int    textLen          = text.length();
    int    currentLineWidth = 0;
    while (wordStart < textLen) {
        int newlinePos = text.indexOf('\n', wordStart);
        if (newlinePos != -1 &&
            (newlinePos < text.indexOf(' ', wordStart) || text.indexOf(' ', wordStart) == -1)) {
            result += text.substring(wordStart, newlinePos + 1);
            wordStart        = newlinePos + 1;
            currentLineWidth = 0;
            continue;
        }
        wordEnd = text.indexOf(' ', wordStart);
        if (wordEnd == -1) { wordEnd = textLen; }
        NString   word    = text.substring(wordStart, wordEnd);
        uint16_t wordLen = tft.textWidth(word);
        if (currentLineWidth + wordLen > tft.width() && currentLineWidth > 0) {
            result += "\n";
            currentLineWidth = 0;
        }
        result += word;
        if (wordEnd < textLen) {
            result += " ";
            currentLineWidth += tft.textWidth(" ");
        }
        currentLineWidth += wordLen;
        wordStart = wordEnd + 1;
    }
    return result;
}

// Function to get the position of a character in a string
// @param str The input string
// @param x reference to the x-coordinate of the character position
// @param y reference to the y-coordinate of the character position
// @param index reference to the index of the character
// @param direction The direction to search
// @param screenWidth The width of the screen
void getCharacterPosition(NString str, int &x, int &y, int &index, int direction = 0,
                          int screenWidth = 240) {
    x                            = 0;
    y                            = 0;
    int              charCount   = 0;
    NString           currentLine = "";
    std::vector<int> lineStarts;
    for (int i = 0; i < str.length(); i++) {
        char currentChar = str[i];
        if (currentChar == '\n') {
            y += tft.fontHeight();
            lineStarts.push_back(i + 1);
            currentLine = "";
        }
        else {
            NString tempLine  = currentLine + currentChar;
            int    lineWidth = tft.textWidth(tempLine);
            if (lineWidth > screenWidth) {
                y += tft.fontHeight();
                lineStarts.push_back(i);
                currentLine = NString(currentChar);
            }
            else { currentLine = tempLine; }
        }
        if (direction == 1 && charCount > index) {
            if (lineStarts.size() > 1) {
                index = lineStarts[lineStarts.size() - 2];
                break;
            }
        }
        else if (direction == 2 && charCount < index) {
            if (lineStarts.size() > 0 && lineStarts.size() - 1 < lineStarts.size()) {
                index = lineStarts[lineStarts.size() - 1];
                break;
            }
        }
        if (charCount == index) {
            int    b             = 0;
            NString lineSubstring = currentLine;
            while (b < lineSubstring.length() &&
                   tft.textWidth(lineSubstring.substring(0, b)) <= screenWidth) {
                b++;
            }
            x = tft.textWidth(lineSubstring.substring(0, b));
            return;
        }
        charCount++;
    }
    x = tft.textWidth(currentLine);
}

// Convert HEX to ASCII
// @param hex The input hexadecimal string
// @return The converted ASCII string
NString HEXTOASCII(NString hex) {
    hex.toUpperCase();
    NString output;

    for (int i = 0; i < hex.length(); i += 2) {
        char h  = hex[i];
        char l  = hex[i + 1];
        int  hv = (h >= '0' && h <= '9') ? h - '0' : h - 'A' + 10;
        int  lv = (l >= '0' && l <= '9') ? l - '0' : l - 'A' + 10;
        char c  = (hv << 4) | lv;
        output += c;
    }
    return output;
}

//  measures text height in pixels
//  @param text The input string
//  @return The height of the text in pixels
int measureStringHeight(const NString &text) {
    int lines      = 1;
    int lineWidth  = 0;
    int spaceWidth = tft.textWidth(" ");
    int lineHeight = tft.fontHeight();

    NString word = "";

    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        if (c == '\r' && i + 1 < text.length() && text[i + 1] == '\n') {
            lines++;
            lineWidth = 0;
            i++;
            continue;
        }

        if (c == '\n' || c == '\r') {
            lines++;
            lineWidth = 0;
            continue;
        }

        if (c == ' ' || i == text.length() - 1) {
            if (i == text.length() - 1 && c != ' ') { word += c; }

            int wordWidth = tft.textWidth(word);

            if (lineWidth + wordWidth > tft.width()) {
                lines++;
                lineWidth = wordWidth + spaceWidth;
            }
            else { lineWidth += wordWidth + spaceWidth; }

            word = "";
        }
        else { word += c; }
    }

    return lines * lineHeight;
}