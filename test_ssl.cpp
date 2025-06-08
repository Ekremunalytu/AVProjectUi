#include <QApplication>
#include <QSslSocket>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "SSL support:" << QSslSocket::supportsSsl();
    qDebug() << "SSL library build version:" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "SSL library version:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "Available SSL backends:" << QSslSocket::availableBackends();
    qDebug() << "Active SSL backend:" << QSslSocket::activeBackend();
    
    // Check for SSL/TLS plugin files
    QDir pluginDir(QApplication::applicationDirPath());
    qDebug() << "Application directory:" << pluginDir.absolutePath();
    
    // Check for SSL-related files
    QStringList filters;
    filters << "*ssl*" << "*tls*" << "*crypto*";
    QStringList sslFiles = pluginDir.entryList(filters, QDir::Files);
    qDebug() << "SSL-related files in app dir:" << sslFiles;
    
    return 0;
}
