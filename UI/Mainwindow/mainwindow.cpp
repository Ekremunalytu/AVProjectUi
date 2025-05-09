#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Pencereyi tam ekran olarak ayarla
    showFullScreen();
}

MainWindow::~MainWindow()
{
    delete ui;
}
