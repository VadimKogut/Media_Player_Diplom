#ifndef THEME_H
#define THEME_H

#include <QString>

struct Theme {
    QString backGroundColor;
    QString buttonColor;
    QString borderColor;
    QString textColor;
    QString labelTextColor;
    QString buttonTextColor;
    QString sliderColor;

    QString borderStyle;

    int borderRadius;
    int borderWidth;
    int buttonWidth;
    int buttonHeight;
};

#endif