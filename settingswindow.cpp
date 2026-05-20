#include "SettingsWindow.h"

#include <QFormLayout>
#include <QColorDialog>
#include <QApplication>
#include <QFile>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QDebug>
#include "ThemeManager.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
{
    ThemeManager manager;
    appTheme = manager.loadFromFile("style.qss");

    setupUI();

    // Устанавливаем фиксированный размер окна по содержимому
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

// ---------------- UI ----------------

QString SettingsWindow::getContrastingTextColor(const QString &bgColor) {
    QColor color(bgColor);

    // Защита от кривого формата цвета
    if (!color.isValid()) return "#000000";

    // Рассчитываем воспринимаемую яркость (Luma) для человеческого глаза
    double luma = 0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();

    if (luma > 128) {
        return "#000000"; // Фон светлый — текст черный
    } else {
        return "#FFFFFF"; // Фон темный — текст белый
    }
}

void SettingsWindow::applyPreset(QString bg, QString btn, QString border, QString text, QString slider)
{
    // Обновляем структуру
    appTheme.backGroundColor = bg;
    appTheme.buttonColor = btn;
    appTheme.borderColor = border;
    appTheme.sliderColor = slider;
    // Переменная text из пресета игнорируется, так как updateTheme()
    // всё равно рассчитает идеальный контрастный цвет автоматически.

    // Сбрасываем некоторые параметры на дефолтные для пресетов
    appTheme.borderWidth = 1;
    appTheme.borderStyle = "solid";
    appTheme.borderRadius = 5;

    // Синхронизируем виджеты без спама сигналами
    this->blockSignals(true);
    sbBorderWidth->setValue(appTheme.borderWidth);
    cbStyle->setCurrentText(appTheme.borderStyle);
    sbRadius->setValue(appTheme.borderRadius);
    this->blockSignals(false);

    // Вызываем финальное обновление и сохранение
    updateTheme();
}

void SettingsWindow::setupUI()
{
    QFormLayout *form = new QFormLayout(this);

    // ---------- PRESET THEMES ----------
    QHBoxLayout *presetLayout = new QHBoxLayout();

    QPushButton *lightBtn = new QPushButton("Light");
    QPushButton *darkBtn  = new QPushButton("Dark");
    QPushButton *oceanBtn = new QPushButton("Ocean");
    QPushButton *sandBtn  = new QPushButton("Sand");

    presetLayout->addWidget(lightBtn);
    presetLayout->addWidget(darkBtn);
    presetLayout->addWidget(oceanBtn);
    presetLayout->addWidget(sandBtn);

    form->addRow("Presets:", presetLayout);

    // ---------- ICON STYLES ----------
    QVBoxLayout *iconStyleLayout = new QVBoxLayout();

    QPushButton *style1Btn = new QPushButton("Regular dark");
    QPushButton *style2Btn = new QPushButton("Pretty dark");
    QPushButton *style3Btn = new QPushButton("Soft Glow");
    QPushButton *style4Btn = new QPushButton("Modern white");

    iconStyleLayout->addWidget(style1Btn);
    iconStyleLayout->addWidget(style2Btn);
    iconStyleLayout->addWidget(style3Btn);
    iconStyleLayout->addWidget(style4Btn);

    form->addRow("Icon Style:", iconStyleLayout);

    // Коннекты для пресетов
    connect(lightBtn, &QPushButton::clicked, this, [this](){
        applyPreset("#FFFFFF", "#F0F0F0", "#CCCCCC", "#000000", "#CCCCCC");
    });
    connect(darkBtn, &QPushButton::clicked, this, [this](){
        applyPreset("#2D2D2D", "#3D3D3D", "#555555", "#FFFFFF", "#555555");
    });
    connect(oceanBtn, &QPushButton::clicked, this, [this](){
        applyPreset("#E3F2FD", "#BBDEFB", "#90CAF9", "#0D47A1", "#90CAF9");
    });
    connect(sandBtn, &QPushButton::clicked, this, [this](){
        applyPreset("#F5F5DC", "#EDE6D3", "#D2B48C", "#4E342E", "#D2B48C");
    });

    // Коннекты для стилей иконок
    connect(style1Btn, &QPushButton::clicked, this, [this](){ emit styleChanged(1); });
    connect(style2Btn, &QPushButton::clicked, this, [this](){ emit styleChanged(2); });
    connect(style3Btn, &QPushButton::clicked, this, [this](){ emit styleChanged(3); });
    connect(style4Btn, &QPushButton::clicked, this, [this](){ emit styleChanged(4); });

    // ---------- COLOR BUTTONS ----------
    bgColorBtn = new QPushButton("Change");
    btnColorBtn = new QPushButton("Change");
    borderColorBtn = new QPushButton("Change");
    sliderColorBtn = new QPushButton("Change");

    connect(bgColorBtn, &QPushButton::clicked, this, &SettingsWindow::chooseBg);
    connect(btnColorBtn, &QPushButton::clicked, this, &SettingsWindow::chooseBtn);
    connect(borderColorBtn, &QPushButton::clicked,this, &SettingsWindow::chooseBorder);
    connect(sliderColorBtn, &QPushButton::clicked, this, &SettingsWindow::chooseSliderColor);

    // ---------- BUTTON WIDTH ----------
    sbWidth = new QSpinBox();
    sbWidth->setRange(29, 80);
    sbWidth->setValue(appTheme.buttonWidth);
    connect(sbWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::updateTheme);

    // ---------- BUTTON HEIGHT ----------
    sbHeight = new QSpinBox();
    sbHeight->setRange(24, 50);
    sbHeight->setValue(appTheme.buttonHeight);
    connect(sbHeight, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::updateTheme);

    // ---------- BORDER WIDTH ----------
    sbBorderWidth = new QSpinBox();
    sbBorderWidth->setRange(0, 6);
    sbBorderWidth->setValue(appTheme.borderWidth);
    connect(sbBorderWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::updateTheme);

    // ---------- BORDER STYLE ----------
    cbStyle = new QComboBox();
    cbStyle->addItems({ "solid", "dashed", "dotted", "double" });
    cbStyle->setCurrentText(appTheme.borderStyle);
    connect(cbStyle, &QComboBox::currentTextChanged, this, &SettingsWindow::updateTheme);

    // ---------- BORDER RADIUS ----------
    sbRadius = new QSpinBox();
    sbRadius->setRange(0, 100);
    sbRadius->setValue(appTheme.borderRadius);
    connect(sbRadius, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWindow::updateTheme);

    // ---------- LAYOUT ----------
    form->addRow("Background color", bgColorBtn);
    form->addRow("Button color", btnColorBtn);
    form->addRow("Border color", borderColorBtn);
    form->addRow("Slider color", sliderColorBtn);

    form->addRow("Button width", sbWidth);
    form->addRow("Button height", sbHeight);

    form->addRow("Border width", sbBorderWidth);
    form->addRow("Border style", cbStyle);
    form->addRow("Border radius", sbRadius);
}

// ---------------- SLOTS ----------------

void SettingsWindow::chooseBg()
{
    QColor c = QColorDialog::getColor(QColor(appTheme.backGroundColor), this);
    if (!c.isValid()) return;

    appTheme.backGroundColor = c.name();
    updateTheme();
}

void SettingsWindow::chooseBtn()
{
    QColor c = QColorDialog::getColor(QColor(appTheme.buttonColor), this);
    if (!c.isValid()) return;

    appTheme.buttonColor = c.name();
    updateTheme();
}

void SettingsWindow::chooseBorder()
{
    QColor c = QColorDialog::getColor(QColor(appTheme.borderColor), this);
    if (!c.isValid()) return;

    appTheme.borderColor = c.name();
    updateTheme();
}

void SettingsWindow::chooseSliderColor()
{
    QColor c = QColorDialog::getColor(QColor(appTheme.sliderColor), this);
    if (!c.isValid()) return;

    appTheme.sliderColor = c.name();
    updateTheme();
}

// ---------------- THEME UPDATE ----------------

void SettingsWindow::updateTheme()
{
    // 1. Считываем ВСЕ значения из UI
    appTheme.buttonWidth = sbWidth->value();
    appTheme.buttonHeight = sbHeight->value();
    appTheme.borderWidth = sbBorderWidth->value();
    appTheme.borderStyle = cbStyle->currentText();
    appTheme.borderRadius = sbRadius->value();

    // 2. АВТОМАТИЧЕСКИЙ РАСЧЕТ ЦВЕТОВ ТЕКСТА
    appTheme.textColor = getContrastingTextColor(appTheme.backGroundColor);
    appTheme.labelTextColor = appTheme.textColor;
    appTheme.buttonTextColor = getContrastingTextColor(appTheme.buttonColor);

    // 3. Авто-расчёт максимального радиуса
    int maxRadius = (appTheme.buttonHeight / 2) - 1;
    if (maxRadius < 0) maxRadius = 0;

    appTheme.borderRadius = qMin(appTheme.borderRadius, maxRadius);

    // 4. Безопасно обновляем UI без рекурсии
    sbRadius->blockSignals(true);
    sbRadius->setValue(appTheme.borderRadius);
    sbRadius->blockSignals(false);

    // 5. Сохранить тему
    ThemeManager manager;
    manager.saveToFile("style.qss", appTheme);

    // 6. Применить стиль
    QFile file("style.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Не удалось открыть файл стилей для применения.";
        return;
    }

    QString style = file.readAll();
    file.close();

    qApp->setStyleSheet(style);
}