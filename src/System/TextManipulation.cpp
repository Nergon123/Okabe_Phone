#include "TextManipulation.h"
unsigned int getIndexOfCount(int count, String input, String str, unsigned int fromIndex) {
    for (int i = 0; i < count; i++) {
        fromIndex = input.indexOf(str, fromIndex + 1);
    }
    return fromIndex;
}


void findSplitPosition(String text, int charIndex, int &posX, int &posY, int direction) {
    int lastNewLine  = 0; 
    int curPosInText = 0; 
    posX             = 0; 
    posY             = 0; 
    for (; curPosInText < charIndex; curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText + 1));
        if (posX >= 240 || text[curPosInText] == '\n') {
            posX = 0; 
            lastNewLine = curPosInText + 1; 
            posY += tft.fontHeight();       
        }
        if (curPosInText >= text.length() - 1) {
            break; 
        }
    }
}

int findCharIndex(String text, int &charIndex, int direction) {
    int prevNL       = 0;             
    int lastNewLine  = 0;             
    int nextNL       = text.length(); 
    int curPosInText = 0;             
    int posX         = 0;             
    int targetX      = 0;             
    for (; curPosInText <= charIndex && curPosInText < text.length(); curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText));
        if (posX >= 240 || text[curPosInText] == '\n') {
            prevNL      = lastNewLine;
            lastNewLine = curPosInText + 1;
            posX        = 0;
        }
        if (curPosInText == charIndex) {
            targetX = posX; 
        }
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
            if (posX >= targetX + 5 || text[curPosInText] == '\n') {
                break;
            }
        }
    }
    else if (direction == DOWN && nextNL < text.length()) {
        lastNewLine = nextNL + 1;
        for (int i = lastNewLine; i < text.length(); i++) {
            if (text[i] == '\n' || tft.textWidth(text.substring(lastNewLine, i + 1)) >= 240 || i == text.length() - 1) {
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

String SplitString(String text) {
    String result           = "";            
    int    wordStart        = 0;             
    int    wordEnd          = 0;             
    int    textLen          = text.length(); 
    int    currentLineWidth = 0;             
    while (wordStart < textLen) {
        int newlinePos = text.indexOf('\n', wordStart);
        if (newlinePos != -1 && (newlinePos < text.indexOf(' ', wordStart) || text.indexOf(' ', wordStart) == -1)) {
            result += text.substring(wordStart, newlinePos + 1);
            wordStart        = newlinePos + 1; 
            currentLineWidth = 0;              
            continue;
        }
        wordEnd = text.indexOf(' ', wordStart);
        if (wordEnd == -1)
            wordEnd = textLen; 
        String   word    = text.substring(wordStart, wordEnd);
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

void getCharacterPosition(String str, int &x, int &y, int &index, int direction = 0, int screenWidth = 240) {
    x                  = 0;  
    y                  = 0;  
    int    charCount   = 0;  
    String currentLine = ""; 
    std::vector<int> lineStarts; 
    for (int i = 0; i < str.length(); i++) {
        char currentChar = str[i];
        if (currentChar == '\n') {
            y += tft.fontHeight();       
            lineStarts.push_back(i + 1); 
            currentLine = ""; 
        } else {
            String tempLine  = currentLine + currentChar;
            int    lineWidth = tft.textWidth(tempLine);
            if (lineWidth > screenWidth) {
                y += tft.fontHeight();             
                lineStarts.push_back(i);           
                currentLine = String(currentChar); 
            } else {
                currentLine = tempLine;
            }
        }
        if (direction == 1 && charCount > index) 
        {
            if (lineStarts.size() > 1) {
                index = lineStarts[lineStarts.size() - 2]; 
                break;                                     
            }
        } else if (direction == 2 && charCount < index) 
        {
            if (lineStarts.size() > 0 && lineStarts.size() - 1 < lineStarts.size()) {
                index = lineStarts[lineStarts.size() - 1]; 
                break;                                     
            }
        }
        if (charCount == index) {
            int    b             = 0;           
            String lineSubstring = currentLine; 
            while (b < lineSubstring.length() && tft.textWidth(lineSubstring.substring(0, b)) <= screenWidth) {
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
String HEXTOASCII(String hex) {
    hex.toUpperCase();
    String output;

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

//  measures text height (duh)
int measureStringHeight(const String &text) {
    int lines      = 1;
    int lineWidth  = 0;
    int spaceWidth = tft.textWidth(" ");
    int lineHeight = tft.fontHeight();

    String word = "";

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
            if (i == text.length() - 1 && c != ' ') {
                word += c;
            }

            int wordWidth = tft.textWidth(word);

            if (lineWidth + wordWidth > tft.width()) {
                lines++;
                lineWidth = wordWidth + spaceWidth;
            } else {
                lineWidth += wordWidth + spaceWidth;
            }

            word = "";
        } else {
            word += c;
        }
    }

    return lines * lineHeight;
}