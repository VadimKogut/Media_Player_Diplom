#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

// ---------------- SAVE ----------------

void ThemeManager::saveToFile(const QString& path, const Theme& current) {
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return;

    QTextStream out(&f);

    // 1. Общие параметры
    out << "QWidget {\n";
    out << "    background-color: " << current.backGroundColor << ";\n";
    out << "    color: " << current.textColor << ";\n";
    out << "}\n\n";

    // 2. Надписи
    out << "QLabel {\n";
    // Убираем !important, он затрудняет парсинг и редко нужен, если селекторы точные
    out << "    color: " << current.labelTextColor << ";\n";
    out << "    background: transparent;\n";
    out << "}\n\n";

    // 3. Кнопки
    out << "QPushButton {\n";
    out << "    font-size: 11px;\n";
    out << "    background-color: " << current.buttonColor << ";\n";
    out << "    color: " << current.buttonTextColor << ";\n";
    out << "    border: " << current.borderWidth << "px " << current.borderStyle << " " << current.borderColor << ";\n";
    out << "    border-radius: " << current.borderRadius << "px;\n";
    // Используем min/max для стабильности в лейаутах
    out << "    min-width: " << current.buttonWidth << "px;\n";
    out << "    min-height: " << current.buttonHeight << "px;\n";
    out << "    padding: 5px;\n";
    out << "}\n\n";

    out << "QPushButton:hover {\n";
    out << "    background-color: rgba(255, 255, 255, 0.15);\n";
    out << "}\n\n";

    // 4. Слайдеры
    out << "QSlider::groove:horizontal {\n"
        << "    height: 6px; background: #C0C0C0; border-radius: 3px;\n"
        << "}\n";
    out << "QSlider::sub-page:horizontal {\n"
        << "    background: " << current.sliderColor << "; border-radius: 3px;\n"
        << "}\n";
    out << "QSlider::handle:horizontal {\n"
        << "    background: #FFFFFF; border: 1px solid " << current.borderColor << ";\n"
        << "    width: 14px; margin: -5px 0; border-radius: 7px;\n"
        << "}\n\n";

    out << "QComboBox {\n";
    out << "    combobox-popup: 1;\n"; // 0 — встроенный список (позволяет открываться вверх), 1 — системный
    out << "    background-color: " << current.buttonColor << ";\n";
    out << "    color: " << current.buttonTextColor << ";\n";
    out << "    border: " << current.borderWidth << "px " << current.borderStyle << " " << current.borderColor << ";\n";
    out << "    border-radius: " << current.borderRadius << "px;\n";
    out << "    padding-left: 5px;\n";
    out << "}\n\n";

    // Стрелочка комбобокса (опционально, для красоты)
    out << "QComboBox::drop-down {\n";
    out << "    border: 0px;\n";
    out << "}\n\n";

    // Сам выпадающий список
    out << "QComboBox QAbstractItemView {\n";
    out << "    border: 1px solid " << current.borderColor << ";\n";
    out << "    background-color: " << current.backGroundColor << ";\n";
    out << "    selection-background-color: " << current.sliderColor << ";\n";
    out << "    selection-color: #FFFFFF;\n";
    out << "    outline: none;\n";
    out << "}\n";
    f.close();
}

// ---------------- LOAD ----------------

Theme ThemeManager::loadFromFile(const QString& path) {
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return Theme();

    QString styleSheet = f.readAll();
    qApp->setStyleSheet(styleSheet); // Сразу применяем ко всему приложению

    f.seek(0);
    QTextStream in(&f);
    Theme t;
    QString currentBlock = "";

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        if (line.contains("{")) {
            currentBlock = line.split("{")[0].trimmed();
            continue;
        }
        if (line == "}") {
            currentBlock = "";
            continue;
        }

        if (line.endsWith(";")) line.chop(1);

        int sep = line.indexOf(':');
        if (sep == -1) continue;

        QString key = line.left(sep).trimmed();
        QString value = line.mid(sep + 1).trimmed();

        // ЧИСТКА ЗНАЧЕНИЯ (Критично для парсинга)
        if (value.contains("!important")) value.replace("!important", "").trimmed();
        if (value.endsWith("px")) value.chop(2);
        value = value.trimmed();

        if (currentBlock == "QWidget") {
            if (key == "background-color") t.backGroundColor = value;
            else if (key == "color") t.textColor = value;
        }
        else if (currentBlock == "QLabel") {
            if (key == "color") t.labelTextColor = value;
        }
        else if (currentBlock == "QPushButton") {
            if (key == "background-color") t.buttonColor = value;
            else if (key == "color") t.buttonTextColor = value;
            else if (key == "border-color") t.borderColor = value;
            else if (key == "border-width") t.borderWidth = value.toInt();
            else if (key == "border-style") t.borderStyle = value;
            else if (key == "border-radius") t.borderRadius = value.toInt();
            // Обрабатываем min-width как ширину
            else if (key == "min-width" || key == "width") t.buttonWidth = value.toInt();
            else if (key == "min-height" || key == "height") t.buttonHeight = value.toInt();
        }
        else if (currentBlock == "QSlider::sub-page:horizontal") {
            if (key == "background") t.sliderColor = value;
        }
    }

    // Заполнение пустот
    if (t.labelTextColor.isEmpty()) t.labelTextColor = t.textColor;
    if (t.buttonTextColor.isEmpty()) t.buttonTextColor = t.textColor;

    return t;
}