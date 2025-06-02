#include "UI/Mainwindow/mainwindow.h"
#include "Database/DatabaseService/DatabaseService.h"
#include "Core/AppConfig.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QDebug>
#include <QDir>

// Add Qt String Literal namespace for Qt 6 compatibility
using namespace Qt::StringLiterals;

/**
 * @brief Application entry point
 * 
 * This function initializes the Qt application, loads configuration,
 * sets up database connection, applies styles and launches the main window.
 * 
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return Application exit code
 */
int main(int argc, char *argv[])
{
    // Initialize Qt application
    QApplication a(argc, argv);

    // Load application configuration using singleton pattern
    auto& appConfig = AppConfig::getInstance();
    appConfig.loadConfig(); // Ensure configuration is loaded
    
    // Initialize database connection through service layer
    auto& dbService = DatabaseService::getInstance();
    if (!dbService.connectDatabase(appConfig.getDatabasePath())) {
        qWarning() << "Failed to connect to database. Some functionality may be limited.";
    }

    // Load and apply application stylesheet
    QFile styleFile(u":/styles/main.qss"_s);
    if (!styleFile.open(QFile::ReadOnly)) {
        qDebug() << "Failed to load style file: " << styleFile.errorString();
        // Print the file path for debugging purposes
        qDebug() << "Searched file: " << styleFile.fileName();
    } else {
        QString styleSheetContent = QString::fromUtf8(styleFile.readAll());
        a.setStyleSheet(styleSheetContent);
        styleFile.close();
    }

    // Create and display the main application window
    MainWindow w(dbService.getDbManager());
    w.show();
    
    // Start the application event loop
    return a.exec();
}