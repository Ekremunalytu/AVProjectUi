#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief The MainWindow class represents the application's main window.
 * 
 * This class is responsible for setting up the main UI components of the application,
 * including navigation and content pages. It serves as the primary container for all
 * other widgets and UI elements.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a MainWindow object.
     * @param parent The parent widget (nullptr by default).
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor that cleans up resources used by MainWindow.
     */
    ~MainWindow();

private:
    Ui::MainWindow *ui; ///< Pointer to the UI form generated from the .ui file
};
#endif // MAINWINDOW_H
