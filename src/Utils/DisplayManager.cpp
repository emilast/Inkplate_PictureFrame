#include "DisplayManager.h"

void initializeDisplay(Inkplate &display, const GFXfont &font)
{
    display.begin();
    display.setFont(&font);
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.setTextWrap(false);
}

void clearAndPrepareDisplay(Inkplate &display)
{
    display.clearDisplay();
}

void drawHeader(Inkplate &display, const GFXfont &font, String text, int xpos, int ypos)
{
    display.setCursor(xpos, ypos);
    display.setFont(&font);
    display.print(text);
    display.setFont(&font);
}

int drawStrikethrough(Inkplate &display, const GFXfont &font, char *text, int xpos, int ypos)
{
    display.setFont(&font);
    display.setCursor(xpos, ypos);
    display.print(text);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, xpos, ypos, &x1, &y1, &w, &h);
    display.fillRect(xpos, ypos - (h / 2), w, 3, BLACK);
    return w;
}

int drawRightString(Inkplate &display, const GFXfont &font, const char *buf, int x, int y, bool strikethrough)
{
    display.setFont(&font);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);
    display.setCursor(x - w, y);
    if (strikethrough)
    {
        display.fillRect(x - w, y - (h / 3), w, 3, BLACK);
    }
    display.print(buf);
    return w;
}

void drawInvertedString(Inkplate &display, const GFXfont &font, const char *buf, int x, int y, int width)
{
    display.setFont(&font);
    int16_t x1, y1;
    uint16_t w, h;
    const int margin = 5;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);
    display.fillRoundRect(x, y - h - margin, width, h + 2 * margin, margin, BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(x + (width / 2) - (w / 2) - 2, y);
    display.print(buf);
    display.setCursor(x + width, y);
    display.setTextColor(BLACK, WHITE);
}

// Helper function to extract words from text
void extractWords(const String &text, String words[], int &wordCount, int maxWords)
{
    wordCount = 0;
    int i = 0;
    while (i < text.length() && wordCount < maxWords)
    {
        int spaceIndex = text.indexOf(' ', i);
        if (spaceIndex == -1)
            spaceIndex = text.length();
        
        if (spaceIndex > i) // Only add non-empty words
        {
            words[wordCount] = text.substring(i, spaceIndex);
            wordCount++;
        }
        i = spaceIndex + 1;
    }
}

int drawWrappedText(Inkplate &display, const GFXfont &font, const String &text, int16_t x, int16_t y, int16_t maxWidth, int lineSpacing, boolean skipRender)
{
    const int spaceWidth = 12; // Approximate width of a space in pixels
    const int maxWords = 100;  // Maximum number of words to handle
    String words[maxWords];
    int wordCount = 0;

    // Extract all words from the text
    extractWords(text, words, wordCount, maxWords);

    display.setFont(&font);
    display.setTextWrap(false);
    
    int linesDrawn = 0;
    int16_t cursor_y = y;
    int16_t cursor_x = x;
    
    // Process words one at a time
    for (int i = 0; i < wordCount; i++)
    {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(words[i], 0, 0, &x1, &y1, &w, &h);
        
        // Check if word fits on current line (with space if not first word on line)
        int16_t wordWidth = w;
        if (cursor_x > x) // Not the first word on the line
        {
            wordWidth += spaceWidth;
        }
        
        // If word doesn't fit and we're not at the start of a line, move to next line
        // if (cursor_x + wordWidth > x + maxWidth && cursor_x > x)
        if (cursor_x + wordWidth > maxWidth && cursor_x > x)
        {
            cursor_y += (font.yAdvance + lineSpacing);
            cursor_x = x;
            linesDrawn++;
        }
        
        // Add space before word if not at start of line
        if (cursor_x > x)
        {
            cursor_x += spaceWidth;
        }
        
        // Render the word
        if (!skipRender)
        {
            display.setCursor(cursor_x, cursor_y);
            display.print(words[i]);
        }
        
        cursor_x += w;
    }
    
    // Count the last line if any words were drawn
    if (wordCount > 0)
    {
        linesDrawn++;
    }
    
    return linesDrawn;
}
