#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include "MainWindow.h"

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type)
    {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtInfoMsg:
        txt = QString("Info: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        break;
    }

    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath); // Ensure the directory exists
    QString logPath = appDataPath + "/debug.log";

    QFile outFile(logPath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << QDateTime::currentDateTime().toString() << " - " << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Install message handler
    qInstallMessageHandler(messageHandler);

    MainWindow window;
    window.show();

    return app.exec();
}