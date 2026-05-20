#include "MediaPlayer.h"
#include "SettingsWindow.h"
#include <QApplication>
#include <QDir>
#include <QEasingCurve>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMediaMetaData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QSettings>
#include <QStyle>
#include <QVBoxLayout>


// --- Реализация кастомного слайдера ---
void ClickableSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int newValue = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().x(), width());
        setValue(newValue);
        emit sliderMoved(newValue);
        event->accept();
    }
    QSlider::mousePressEvent(event);
}

// --- Основной класс плеера ---

MediaPlayer::MediaPlayer(QWidget *parent) : QMainWindow(parent) {
    this->setObjectName("MainWindow");

    // Инициализируем важные переменные сразу!
    m_lastVolume = 70;
    m_isRepeated = false;
    m_currentTrackIndex = -1;

    setMouseTracking(true);
    setWindowTitle("Z-Player");
    resize(1000, 600);

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    videoWidget = new QVideoWidget(this);
    videoWidget->setMouseTracking(true);
    player->setVideoOutput(videoWidget);
    player->setAudioOutput(audioOutput);

    centralWidget = new QWidget(this);
    centralWidget->setMouseTracking(true);
    setCentralWidget(centralWidget);

    setupUI();
    loadStyleSheet();

    controlsTimer = new QTimer(this);
    controlsTimer->setInterval(3000);

    setupConnections();

    QSettings settings("MyApps", "Z-Player");
    int savedStyle = settings.value("iconStyle", 1).toInt();
    onChangeStyle(savedStyle);

    QString appDir = QCoreApplication::applicationDirPath();
    QString iconPath = QDir(appDir).filePath("app_icon.png");
    setWindowIcon(QIcon(iconPath));
    // Даем окну фокус при старте
    this->setFocus();

    videoWidget->setAttribute(Qt::WA_Hover);
    videoWidget->setMouseTracking(true);

}

void MediaPlayer::setupUI() {
    // --- 1. ВЕРХНЯЯ ПАНЕЛЬ ---
    topPanel = new QWidget(centralWidget);
    topPanel->setFocusPolicy(Qt::NoFocus);
    topPanel->setObjectName("topPanel");

    QHBoxLayout *topLayout = new QHBoxLayout(topPanel);
    topLayout->setContentsMargins(10, 5, 10, 5);

    openFileBtn = new QPushButton("File", topPanel);
    helpBtn = new QPushButton("Guide", topPanel);
    aboutBtn = new QPushButton("About", topPanel);
    fileNameLabel = new QLabel("File name : No file", topPanel);
    fileNameLabel->setStyleSheet("font-weight: bold; margin-left: 15px; font-size: 13px;");

    topLayout->addWidget(openFileBtn);
    topLayout->addWidget(helpBtn);
    topLayout->addWidget(aboutBtn);
    topLayout->addWidget(fileNameLabel);
    topLayout->addStretch();

    // --- 2. НИЖНЯЯ ПАНЕЛЬ ---
    controlsWidget = new QWidget(centralWidget);
    controlsWidget->setFocusPolicy(Qt::NoFocus);
    controlsWidget->setObjectName("controlsWidget");

    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(10, 5, 10, 10);

    QHBoxLayout *seekLayout = new QHBoxLayout();
    timeLabel = new QLabel("00:00 / 00:00", controlsWidget);
    seekSlider = new ClickableSlider(Qt::Horizontal, controlsWidget);
    seekLayout->addWidget(timeLabel);
    seekLayout->addWidget(seekSlider);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QSize standardIconSize(32, 32);

    shuffleBtn = new QPushButton(controlsWidget);
    shuffleBtn->setCheckable(true);
    shuffleBtn->setIconSize(standardIconSize);

    prevBtn = new QPushButton(controlsWidget);
    prevBtn->setIconSize(standardIconSize);

    back10Btn = new QPushButton(controlsWidget);
    back10Btn->setIconSize(standardIconSize);

    playPauseBtn = new QPushButton(controlsWidget);
    playPauseBtn->setIconSize(standardIconSize);

    forward10Btn = new QPushButton(controlsWidget);
    forward10Btn->setIconSize(standardIconSize);

    nextBtn = new QPushButton(controlsWidget);
    nextBtn->setIconSize(standardIconSize);

    loopBtn = new QPushButton(controlsWidget);
    loopBtn->setCheckable(true);
    loopBtn->setIconSize(standardIconSize);

    muteBtn = new QPushButton(controlsWidget);
    muteBtn->setFocusPolicy(Qt::NoFocus);

    volumeSlider = new QSlider(Qt::Horizontal, controlsWidget);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(70);
    volumeSlider->setFixedWidth(100);

    // --- НОВЫЙ БЛОК: ВЫБОРА СКОРОСТИ (COMBOBOX) ---
    speedCombo = new QComboBox(controlsWidget);
    speedCombo->setFixedWidth(65);
    speedCombo->setFocusPolicy(Qt::NoFocus);

    QList<double> speeds = {0.25, 0.5, 1.0, 1.25, 1.5, 2.0, 4.0};
    for(double s : speeds) {
        speedCombo->addItem(QString("%1x").arg(s), s);
    }
    speedCombo->setCurrentIndex(2); // Устанавливаем 1.0x по умолчанию

    connect(speedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        double s = speedCombo->itemData(index).toDouble();
        player->setPlaybackRate(s);
    });

    subtitleBtn = new QPushButton(controlsWidget);
    subtitleBtn->setIconSize(standardIconSize);
    subtitleBtn->setCheckable(true);

    settingsBtn = new QPushButton(controlsWidget);
    settingsBtn->setIconSize(standardIconSize);

    fullscreenBtn = new QPushButton(controlsWidget);
    fullscreenBtn->setIconSize(QSize(30,30));

    // Сборка кнопок в ряд
    buttonsLayout->addWidget(shuffleBtn);
    buttonsLayout->addWidget(prevBtn);
    buttonsLayout->addWidget(back10Btn);
    buttonsLayout->addWidget(playPauseBtn);
    buttonsLayout->addWidget(forward10Btn);
    buttonsLayout->addWidget(nextBtn);
    buttonsLayout->addWidget(loopBtn);
    buttonsLayout->addSpacing(15);
    buttonsLayout->addWidget(muteBtn);
    buttonsLayout->addWidget(volumeSlider);

    buttonsLayout->addSpacing(15);
     // Добавляем выпадающий список

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(speedCombo);
    buttonsLayout->addWidget(subtitleBtn);
    buttonsLayout->addWidget(settingsBtn);
    buttonsLayout->addWidget(fullscreenBtn);

    controlsLayout->addLayout(seekLayout);
    controlsLayout->addLayout(buttonsLayout);

    // --- 3. ГЛАВНАЯ КОМПОНОВКА ОКНА ---
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topPanel);
    mainLayout->addWidget(videoWidget, 1);
    mainLayout->addWidget(controlsWidget);

    // --- 4. ФИНАЛЬНЫЕ НАСТРОЙКИ И ТАЙМЕРЫ ---
    this->setMouseTracking(true);
    centralWidget->setMouseTracking(true);
    videoWidget->setMouseTracking(true);
    topPanel->setMouseTracking(true);
    controlsWidget->setMouseTracking(true);

    QList<QPushButton*> allBtns = {
        muteBtn, subtitleBtn, openFileBtn, helpBtn, aboutBtn, shuffleBtn, prevBtn,
        back10Btn, playPauseBtn, forward10Btn, nextBtn,
        loopBtn, settingsBtn, fullscreenBtn
    };

    for (QPushButton* btn : allBtns) {
        btn->setFocusPolicy(Qt::NoFocus);
        btn->installEventFilter(this);
    }

    videoWidget->installEventFilter(this);
    centralWidget->installEventFilter(this);
    topPanel->installEventFilter(this);
    controlsWidget->installEventFilter(this);

    // --- ПРОВЕРКА КООРДИНАТ КУРСОРA ---
    lastMousePos = QCursor::pos();
    mouseCheckTimer = new QTimer(this);
    connect(mouseCheckTimer, &QTimer::timeout, this, [this]() {
        QPoint currentPos = QCursor::pos();
        if (currentPos != lastMousePos) {
            lastMousePos = currentPos;
            showControls();
        }
    });
    mouseCheckTimer->start(100);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();
}

void MediaPlayer::setupConnections() {
    QList<QPushButton*> animatedButtons = {
        openFileBtn, helpBtn, aboutBtn, shuffleBtn, prevBtn, back10Btn,
        playPauseBtn, forward10Btn, nextBtn, loopBtn,
        settingsBtn, fullscreenBtn,muteBtn, subtitleBtn
    };

    for (QPushButton* btn : animatedButtons) {
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            playClickAnimation(btn);
        });
    }

    connect(openFileBtn, &QPushButton::clicked, this, &MediaPlayer::openFile);
    connect(helpBtn, &QPushButton::clicked, this, &MediaPlayer::showHelp);
    connect(aboutBtn, &QPushButton::clicked, this, &MediaPlayer::showAbout);
    connect(settingsBtn, &QPushButton::clicked, this, &MediaPlayer::showSettings);
    connect(playPauseBtn, &QPushButton::clicked, this, &MediaPlayer::togglePlayback);
    connect(fullscreenBtn, &QPushButton::clicked, this, &MediaPlayer::toggleFullscreen);

    connect(back10Btn, &QPushButton::clicked, this, [this](){
        player->setPosition(qMax<qint64>(0, player->position() - 10000));
    });
    connect(forward10Btn, &QPushButton::clicked, this, [this](){
        player->setPosition(qMin<qint64>(player->duration(), player->position() + 10000));
    });

    connect(nextBtn, &QPushButton::clicked, this, &MediaPlayer::nextTrack);
    connect(prevBtn, &QPushButton::clicked, this, &MediaPlayer::prevTrack);

    connect(loopBtn, &QPushButton::toggled, this, [this](bool checked){
        m_isRepeated = checked;
    });

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia) {
            if (m_isRepeated) playCurrentIndex();
            else nextTrack();
        }
    });

    connect(player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayer::updateState);
    connect(player, &QMediaPlayer::durationChanged, this, &MediaPlayer::updateDuration);
    connect(player, &QMediaPlayer::positionChanged, this, &MediaPlayer::updatePosition);
    connect(seekSlider, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
    connect(subtitleBtn, &QPushButton::clicked, this, &MediaPlayer::toggleSubtitles);
    connect(volumeSlider, &QSlider::valueChanged, this, [this](int v){
        audioOutput->setVolume(v / 100.0);
        updateIcons();
    });

    connect(controlsTimer, &QTimer::timeout, this, &MediaPlayer::hideControls);

    connect(muteBtn, &QPushButton::clicked, this, [this]() {
        if (volumeSlider->value() > 0) {
            m_lastVolume = volumeSlider->value();
            volumeSlider->setValue(0);
        } else {
            volumeSlider->setValue(m_lastVolume > 0 ? m_lastVolume : 70);
        }
    });
}

// --- Логика воспроизведения ---

void MediaPlayer::openFile() {
    QString filter = "Media Files (*.mp4 *.mkv *.avi *.mov *.mp3 *.wav *.ogg *.flac);;All Files (*)";
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", filter);

    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        QDir dir = fileInfo.absoluteDir();
        QString ext = fileInfo.suffix();

        QStringList nameFilters;
        nameFilters << "*." + ext;
        QFileInfoList entryList = dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);

        m_playlist.clear();
        for (const QFileInfo &info : entryList) {
            m_playlist.append(info.absoluteFilePath());
        }

        m_currentTrackIndex = m_playlist.indexOf(filePath);
        if (m_currentTrackIndex == -1) m_currentTrackIndex = 0;

        playCurrentIndex();
    }
    this->setFocus(); // Возвращаем фокус после закрытия диалога
}

void MediaPlayer::playCurrentIndex() {
    if (m_playlist.isEmpty()) return;

    QString filePath = m_playlist[m_currentTrackIndex];
    QFileInfo fileInfo(filePath);
    fileNameLabel->setText(QString("File name: %1").arg(fileInfo.fileName()));

    QString ext = fileInfo.suffix().toLower();
    bool isAudio = QStringList({"mp3", "wav", "ogg", "flac"}).contains(ext);

    if (isAudio) {
        // 1. Скрываем лишнее
        videoWidget->hide();
        fullscreenBtn->hide();

        this->setFixedSize(this->layout()->sizeHint());

    } else {
        // 3. Возвращаем режим видео
        videoWidget->show();
        fullscreenBtn->show();

        // 4. Снимаем ограничения на размер
        this->setMinimumSize(900, 500); // Твой стандартный минимум
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // Снимаем блок

        // Если окно было слишком маленьким после музыки — увеличиваем
        if (this->height() < 500) {
            this->resize(1000, 600);
        }
    }

    player->setSource(QUrl::fromLocalFile(filePath));
    player->play();
}

void MediaPlayer::togglePlayback() {
    if (player->playbackState() == QMediaPlayer::PlayingState) player->pause();
    else player->play();
    this->setFocus(); // Важно!
}

// --- UI логика ---

void MediaPlayer::showControls() {
    // Если панели спрятаны — проявляем
    if (controlsWidget->isHidden()) {
        controlsWidget->show();
        topPanel->show();
        this->setCursor(Qt::ArrowCursor);
    }

    // Сбрасываем 3-секундный таймер исчезновения
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        controlsTimer->start(3000);
    }
}

void MediaPlayer::hideControls() {
    // Если мышь над панелью, не прячем, а даем еще 3 секунды
    if (controlsWidget->underMouse() || topPanel->underMouse()) {
        controlsTimer->start(3000);
        return;
    }

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        controlsWidget->hide();
        topPanel->hide();

        if (this->isFullScreen()) {
            this->setCursor(Qt::BlankCursor);
        }
    }
}

bool MediaPlayer::eventFilter(QObject *obj, QEvent *event) {
    // 1. Клик мыши или двойной клик (движение теперь ловит фоновый таймер координат)
    // Оставляем это здесь, чтобы панели мгновенно реагировали на нажатия
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick) {

        showControls(); // Показываем и сбрасываем таймер на 3 сек
    }

    // 2. Логика анимации кнопок (Enter/Leave)
    // Работает отлично, оставляем без изменений
    if (QPushButton *btn = qobject_cast<QPushButton*>(obj)) {
        if (event->type() == QEvent::Enter) {
            QPropertyAnimation *anim = new QPropertyAnimation(btn, "iconSize");
            anim->setDuration(150);
            anim->setEndValue(QSize(36, 36));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        } else if (event->type() == QEvent::Leave) {
            QPropertyAnimation *anim = new QPropertyAnimation(btn, "iconSize");
            anim->setDuration(150);
            anim->setEndValue(QSize(32, 32));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    // 3. Специфичная логика для видео (пауза по клику)
    if (obj == videoWidget && event->type() == QEvent::MouseButtonPress) {
        togglePlayback();
        // Мы НЕ возвращаем true, чтобы событие MouseButtonPress
        // дошло до пункта №1 и вызвало showControls()
    }

    return QMainWindow::eventFilter(obj, event);
}

void MediaPlayer::keyPressEvent(QKeyEvent *event) {
    int key = event->key();

    // Мы проверяем и английскую клавишу, и её русский аналог на стандартной раскладке
    switch (key) {

    // Пауза (Пробел — он везде пробел, обычно работает всегда)
    case Qt::Key_Space:
        togglePlayback();
        break;

    // Полный экран (F или русская А)
    case Qt::Key_F:
    case 0x0410: // Код русской 'А'
        toggleFullscreen();
        break;

    // Перемотка назад (Стрелки обычно не зависят от языка, но проверим)
    case Qt::Key_Left:
        player->setPosition(qMax<qint64>(0, player->position() - 10000));
        break;

    case Qt::Key_Right:
        player->setPosition(qMin<qint64>(player->duration(), player->position() + 10000));
        break;

    // Громкость
    case Qt::Key_Up:
        volumeSlider->setValue(volumeSlider->value() + 5);
        break;
    case Qt::Key_Down:
        volumeSlider->setValue(volumeSlider->value() - 5);
        break;

    // Следующий трек (N или русская Т)
    case Qt::Key_N:
    case 0x0422: // Код русской 'Т'
        nextTrack();
        break;

    // Предыдущий трек (P или русская З)
    case Qt::Key_P:
    case 0x0417: // Код русской 'З'
        prevTrack();
        break;

    // Mute (M или русская Ь)
    case Qt::Key_M:
    case 0x042c: // Код русской 'Ь'
        if (volumeSlider->value() > 0) {
            m_lastVolume = volumeSlider->value();
            volumeSlider->setValue(0);
        } else {
            volumeSlider->setValue(m_lastVolume > 0 ? m_lastVolume : 70);
        }
        break;

    case Qt::Key_Escape:
        if (isFullScreen()) showNormal();
        break;

    default:
        QMainWindow::keyPressEvent(event);
    }
}

// Прочие методы (nextTrack, prevTrack, updateIcons и т.д. остаются без изменений)...
// Но не забудь добавить в showAbout() и showHelp() возврат фокуса, как в твоем коде.

void MediaPlayer::showAbout() {
    QMessageBox::information(this, "About Developer",
                             "This Media Player was developed as part of a graduation"
                             "this is project by Vadim Stanislavovich Kogut,  "
                             "a student of the Moscow Aviation Institute, group M3O-433B-22.");
    this->setFocus();
}

void MediaPlayer::showHelp() {
    QString helpText =
        "<h3 style='color: #0984e3;'>Z-Player Quick Start Guide</h3>"
        "<p><b>Global Hotkeys (Layout Independent):</b></p>"
        "<ul>"
        "<li><b>Spacebar</b> — Play / Pause playback</li>"
        "<li><b>F</b> — Toggle Fullscreen mode</li>"
        "<li><b>Esc</b> — Exit Fullscreen</li>"
        "<li><b>Left / Right Arrows</b> — Seek backward/forward (10 sec)</li>"
        "<li><b>Up / Down Arrows</b> — Increase / Decrease volume</li>"
        "<li><b>N / P</b> — Next / Previous track</li>"
        "<li><b>M</b> — Mute / Unmute audio</li>"
        "</ul>"
        "<p><b>Playback Features:</b></p>"
        "<ul>"
        "<li><b>Shuffle</b> — Randomize playback order from the folder</li>"
        "<li><b>Loop</b> — Repeat the current media file</li>"
        "<li><b>Subtitles</b> — Toggle available subtitle tracks (20pt font)</li>"
        "<li><b>Settings</b> — Customize UI icon styles and themes</li>"
        "</ul>"
        "<hr>"
        "<p><small>Developed for MAI Graduation Thesis Project.</small></p>";

    QMessageBox helpBox(this);
    helpBox.setWindowTitle("User Guide");
    helpBox.setTextFormat(Qt::RichText);
    helpBox.setText(helpText);
    helpBox.setIcon(QMessageBox::Question); // Иконка вопроса выглядит уместно для гайда
    helpBox.addButton("Got it", QMessageBox::AcceptRole);

    helpBox.exec();

    // Возврат фокуса для мгновенной работы горячих клавиш
    this->activateWindow();
    this->setFocus();
}

void MediaPlayer::nextTrack() {
    if (m_playlist.isEmpty()) return;
    if (shuffleBtn->isChecked()) {
        m_currentTrackIndex = QRandomGenerator::global()->bounded(m_playlist.size());
    } else {
        m_currentTrackIndex = (m_currentTrackIndex + 1) % m_playlist.size();
    }
    playCurrentIndex();
}

void MediaPlayer::prevTrack() {
    if (m_playlist.isEmpty()) return;
    m_currentTrackIndex = (m_currentTrackIndex - 1 + m_playlist.size()) % m_playlist.size();
    playCurrentIndex();
}

void MediaPlayer::toggleFullscreen() {
    if (videoWidget->isHidden()) return;
    if (isFullScreen()) showNormal();
    else showFullScreen();
}

void MediaPlayer::updateIcons() {
    auto getIcon = [&](QString name) { return QIcon(m_currentStylePath + name); };
    shuffleBtn->setIcon(getIcon("shuffle.png"));
    prevBtn->setIcon(getIcon("prev.png"));
    back10Btn->setIcon(getIcon("back10.png"));
    playPauseBtn->setIcon(player->playbackState() == QMediaPlayer::PlayingState ? getIcon("pause.png") : getIcon("play.png"));
    forward10Btn->setIcon(getIcon("forward10.png"));
    nextBtn->setIcon(getIcon("next.png"));
    loopBtn->setIcon(getIcon("loop.png"));
    settingsBtn->setIcon(getIcon("settings.png"));
    fullscreenBtn->setIcon(getIcon("fullscreen.png"));
    subtitleBtn->setIcon(getIcon("subtitles.png"));

    if (volumeSlider->value() == 0) {
        muteBtn->setIcon(getIcon("mute.png")); // Убедитесь, что файл mute.png есть в папках стилей
    } else {
        muteBtn->setIcon(getIcon("volume.png"));
    }
}

void MediaPlayer::updateState(QMediaPlayer::PlaybackState state) {
    updateIcons(); // Обновит плей/паузу
    if (state == QMediaPlayer::PlayingState) controlsTimer->start();
    else showControls();
}

void MediaPlayer::updateDuration(qint64 duration) {
    m_duration = duration;
    seekSlider->setRange(0, duration);
}

void MediaPlayer::updatePosition(qint64 position) {
    if (!seekSlider->isSliderDown()) seekSlider->setValue(position);
    bool hasHours = m_duration >= 3600000;
    timeLabel->setText(formatTime(position, hasHours) + " / " + formatTime(m_duration, hasHours));
}

void MediaPlayer::showSettings() {
    if (!settingsWindow) {
        settingsWindow = new SettingsWindow(this);
        settingsWindow->setWindowFlags(Qt::Window);
        connect(settingsWindow, &SettingsWindow::styleChanged, this, &MediaPlayer::onChangeStyle);
    }
    settingsWindow->show();
}

void MediaPlayer::mouseMoveEvent(QMouseEvent *event) {
    showControls();
    QMainWindow::mouseMoveEvent(event);
}

QString MediaPlayer::formatTime(qint64 ms, bool showHours) {
    qint64 totalSeconds = ms / 1000;
    int seconds = totalSeconds % 60;
    int minutes = (totalSeconds / 60) % 60;
    int hours = totalSeconds / 3600;
    if (showHours) return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

void MediaPlayer::loadStyleSheet() {
    QFile file("style.qss");
    if (file.open(QFile::ReadOnly)) qApp->setStyleSheet(file.readAll());
}

void MediaPlayer::onChangeStyle(int styleNumber) {
    QSettings settings("MyApps", "Z-Player");
    settings.setValue("iconStyle", styleNumber);
    m_currentStylePath = QDir::toNativeSeparators(QString("button_styles/%1/").arg(styleNumber));
    updateIcons();
}

QPixmap MediaPlayer::getPixmap(const QString &name, QSize size) {
    QPixmap pix(m_currentStylePath + name);
    if (pix.isNull()) { pix = QPixmap(size); pix.fill(Qt::transparent); }
    return pix.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void MediaPlayer::playClickAnimation(QPushButton* btn) {
    if (!btn) return;
    QPropertyAnimation *anim = new QPropertyAnimation(btn, "iconSize");
    anim->setDuration(200);
    anim->setStartValue(btn->iconSize());
    anim->setKeyValueAt(0.3, QSize(26, 26));
    anim->setEndValue(QSize(32, 32));
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MediaPlayer::openFileDirectly(const QString &filePath) {
    if (filePath.isEmpty()) return;

    // Очищаем путь от лишних символов и проверяем существование
    QFileInfo fileInfo(QDir::cleanPath(filePath));
    if (!fileInfo.exists() || !fileInfo.isFile()) return;

    QDir dir = fileInfo.absoluteDir();
    QString ext = fileInfo.suffix().toLower();

    // Сканируем папку, чтобы составить плейлист из файлов того же типа
    QStringList nameFilters;
    nameFilters << "*." + ext;

    QFileInfoList entryList = dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    m_playlist.clear();
    for (const QFileInfo &info : entryList) {
        m_playlist.append(info.absoluteFilePath());
    }

    // Находим индекс открытого файла в созданном плейлисте
    m_currentTrackIndex = m_playlist.indexOf(fileInfo.absoluteFilePath());
    if (m_currentTrackIndex == -1) m_currentTrackIndex = 0;

    // Запускаем воспроизведение
    playCurrentIndex();
}

void MediaPlayer::toggleSubtitles() {
    // Получаем список всех доступных дорожек субтитров
    QMediaMetaData metaData = player->metaData();
    auto tracks = player->subtitleTracks();

    if (tracks.isEmpty()) {
        QMessageBox::information(this, "Subtitles", "No subtitle tracks found in this file.");
        subtitleBtn->setChecked(false);
        return;
    }

    // Логика: если сейчас субтитры выключены (-1), включаем первую дорожку (0)
    // Если включены — выключаем.
    if (player->activeSubtitleTrack() == -1) {
        player->setActiveSubtitleTrack(0); // Включаем первую дорожку
        subtitleBtn->setChecked(true);
    } else {
        player->setActiveSubtitleTrack(-1); // Выключаем
        subtitleBtn->setChecked(false);
    }
}

MediaPlayer::~MediaPlayer() {}