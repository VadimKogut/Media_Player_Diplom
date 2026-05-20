#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QMap>
#include "theme.h"

class ThemeManager{
public:

    void saveToFile  (const QString& path, const Theme& current);
    Theme loadFromFile(const QString& path);

};

#endif