#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#pragma once

#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

#include "theme.h"

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);

private slots:
    void chooseBg();
    void chooseBtn();
    void chooseBorder();
    void updateTheme();
    void chooseSliderColor();
    QString getContrastingTextColor(const QString &bgColor);

    // Слот для применения готовых шаблонов (пресетов)
    void applyPreset(QString bg, QString btn, QString border, QString text, QString slider);
signals:
    void styleChanged(int styleNumber);
private:
    // Текущая редактируемая тема
    Theme appTheme;

    // Элементы UI
    QSpinBox *sbWidth = nullptr;
    QSpinBox *sbHeight = nullptr;
    QSpinBox *sbBorderWidth = nullptr;
    QSpinBox *sbRadius = nullptr;

    QComboBox *cbStyle = nullptr;

    QPushButton *bgColorBtn = nullptr;
    QPushButton *btnColorBtn = nullptr;
    QPushButton *borderColorBtn = nullptr;
    QPushButton *sliderColorBtn = nullptr;
    // Кнопки для пресетов
    QPushButton *lightBtn = nullptr;
    QPushButton *darkBtn  = nullptr;
    QPushButton *oceanBtn = nullptr;
    QPushButton *sandBtn  = nullptr;

    // Функции
    void setupUI();
};

#endif // SETTINGSWINDOW_H