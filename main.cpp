#include <QApplication>
#include <QCommandLineParser>
#include "MediaPlayer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Z-Player");
    app.setOrganizationName("Z-Company");
    app.setApplicationVersion("1.0");

    MediaPlayer player;
    player.show();

    // Настраиваем парсер командной строки
    QCommandLineParser parser;
    parser.setApplicationDescription("Z-Player Media Player");
    parser.addHelpOption();
    parser.addVersionOption();

    // Добавляем возможность принимать позиционный аргумент (путь к файлу)
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        // Берем первый аргумент как путь к файлу
        player.openFileDirectly(args.at(0));
    }

    return app.exec();
}