#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

// 引入QT网络编程相关的头文件
#include <QTcpSocket>
#include <QHostAddress>
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEdit_inputRejected()
{

}

