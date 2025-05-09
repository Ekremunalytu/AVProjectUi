#include "UI/Mainwindow/mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QFile styleFile(":/styles/main.qss");
if (!styleFile.open(QFile::ReadOnly)) {
    qDebug() << "Stil dosyası yüklenemedi: " << styleFile.errorString();
    // Hata ayıklama için dosya yolunu da yazdır
    qDebug() << "Aranan dosya: " << styleFile.fileName();
} else {
    QString styleSheetContent = styleFile.readAll();
    a.setStyleSheet(styleSheetContent);
    styleFile.close();
}

    MainWindow w;
    w.show();
    return a.exec();
}