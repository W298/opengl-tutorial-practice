#ifndef TEXT2D_HPP
#define TEXT2D_HPP

void initText2D(const char * texturePath, int winWidth, int winHeight);
void printText2D(const char * text, int x, int y, int size);
void cleanupText2D();

#endif