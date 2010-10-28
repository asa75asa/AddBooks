#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonDBDir_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory with database files"),
                                                   "",
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  ui->editDBDir->setText(dir);
}

void MainWindow::on_buttonNewDir_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory with new files"),
                                                   "",
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  ui->editNewDir->setText(dir);
}

void MainWindow::on_buttonStart_clicked()
{

}
