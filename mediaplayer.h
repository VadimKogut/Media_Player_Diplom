#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QStringList>
#include <QComboBox>

class SettingsWindow;

// Кастомный слайдер для удобной перемотки кликом
class ClickableSlider : public QSlider {
    Q_OBJECT
public:
    explicit ClickableSlider(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSlider(orientation, parent) {}
protected:
    void mousePressEvent(QMouseEvent *event) override;
};

class MediaPlayer : public QMainWindow {
    Q_OBJECT

public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    void openFileDirectly(const QString &filePath);
    ~MediaPlayer();

private slots:
    // Основные действия
    void openFile();
    void showAbout();
    void togglePlayback();
    void toggleFullscreen();
    void showSettings();

    // Управление плейлистом и перемоткой
    void nextTrack();
    void prevTrack();
    void skipForward();  // +10 сек
    void skipBackward(); // -10 сек

    // Обновление состояния (синхронизация с QMediaPlayer)
    void updateState(QMediaPlayer::PlaybackState state);
    void updateDuration(qint64 duration);
    void updatePosition(qint64 position);

    // UI логика
    void hideControls();
    void showControls();
    void loadStyleSheet();

    void onChangeStyle(int styleNumber);
    void toggleSubtitles();

protected:
    // Обработка событий мыши для скрытия/показа панелей
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void playClickAnimation(QPushButton* btn);
    void keyPressEvent(QKeyEvent *event) override;
    void showHelp();

private:
    // Вспомогательные методы инициализации
    void setupUI();
    void setupConnections();
    void playCurrentIndex();
    QPoint lastMousePos; // Храним последнюю позицию (QPoint удобнее, чем два int)
    QTimer *mouseCheckTimer;
    QString formatTime(qint64 ms, bool showHours);

    // Ядро мультимедиа
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QVideoWidget *videoWidget;

    // Контейнеры интерфейса
    QWidget     *centralWidget;
    QWidget     *controlsWidget;

    // Кнопки управления
    QPushButton *openFileBtn;
    QPushButton *helpBtn;
    QPushButton *aboutBtn;
    QPushButton *playPauseBtn;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QPushButton *back10Btn;
    QPushButton *forward10Btn;
    QPushButton *shuffleBtn;
    QPushButton *loopBtn;
    QPushButton *fullscreenBtn;
    QPushButton *settingsBtn;
    QPushButton *subtitleBtn;
    QWidget *topPanel;
    QPushButton *muteBtn;         // Вместо volumeLabel
    QLabel *fileNameLabel;        // Для названия файла
    QComboBox *speedCombo;

    // Элементы индикации и ввода
    QSlider     *seekSlider;
    QSlider     *volumeSlider;
    QLabel      *timeLabel;
    QLabel      *volumeLabel;

    // Состояние плеера
    QStringList m_playlist;
    bool m_isRepeated = false;
    int m_currentTrackIndex = 0;
    qint64 m_duration = 0;
    int m_lastVolume = 70;
    QTimer *controlsTimer;
    SettingsWindow *settingsWindow = nullptr;

    QString m_currentStylePath = "button_styles/1/";
    void updateIcons();
    QPixmap getPixmap(const QString &name, QSize size = QSize(32, 32));
};

#endif // MEDIAPLAYER_H