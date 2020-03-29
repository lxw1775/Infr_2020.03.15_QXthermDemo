#include "./UI/mainwindow.h"
#include <QTranslator>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    bool b = false;
    b = translator.load("Language/QXthermDemo_zh_CN.ts");
    a.installTranslator(&translator);
    MainWindow w;
    w.show();
    return a.exec();
}

/*
void LHSyncClientPrivate::InitUiByLanguage(const QString strLanguage)
{
    if (strLanguage.isEmpty())
    {
        return;
    }

    QString strLanguageFile;
    if (strLanguage.compare("en") == 0)
    {
        strLanguageFile = qApp->applicationDirPath() + QString("/languages/%1/%2").arg(LHT_SYNCCLIENT_VERSION_PRODOCUTNAME).arg(LHT_SYNCCLIENT_EN_FILE);
    }
    else if (strLanguage.compare("zh") == 0)
    {
        strLanguageFile = qApp->applicationDirPath() + QString("/languages/%1/%2").arg(LHT_SYNCCLIENT_VERSION_PRODOCUTNAME).arg(LHT_SYNCCLIENT_ZH_FILE);
    }

    if (QFile(strLanguageFile).exists())
    {
        m_translator->load(strLanguageFile);
        qApp->installTranslator(m_translator);
    }
    else
    {
        qDebug() << "[houqd] authclient language file does not exists ...";
    }
}
*/
