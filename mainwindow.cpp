#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pSettings = new QSettings(tr("Alexey Sinitsyn"), tr("AddBooks"));
    bStarted = false;
    loadSettings();
}

MainWindow::~MainWindow()
{
  delete pSettings;
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
  if (bStarted)
  {
    ui->buttonStart->setText(tr("Start"));
    thread.stop();

  }
  else
  {
    ui->buttonStart->setText(tr("Stop"));
    saveSettings();
    if (thread.setData(ui->editNewDir->text(),
                   ui->editDBDir->text(),
                   tr("localhost"),
                   ui->editDatabase->text(),
                   ui->editUsername->text(),
                   ui->editPassword->text(),
                   ui->checkCompress->isChecked(),
                   true,
                   true))
    {
      thread.run();
    }



  }
  bStarted = !bStarted;

}

void MainWindow::saveSettings()
{
  pSettings->setValue(tr("Settings/NewDir"), ui->editNewDir->text());
  pSettings->setValue(tr("Settings/DBDir"), ui->editDBDir->text());
  pSettings->setValue(tr("Settings/Compress"), ui->checkCompress->isChecked());
  pSettings->setValue(tr("Settings/SkipDuplicates"), ui->checkDuplicates->isChecked());
  pSettings->setValue(tr("Settings/Database"), ui->editDatabase->text());
  pSettings->setValue(tr("Settings/Username"), ui->editUsername->text());
  pSettings->setValue(tr("Settings/Password"), ui->editPassword->text());
  pSettings->setValue(tr("Settings/HostName"), sHostName);
}

void MainWindow::loadSettings()
{
  QString text;
  text = pSettings->value(tr("Settings/NewDir"), tr("")).toString();
  if (text.length() > 0)
  {
    ui->editNewDir->setText(text);
  }
  text = pSettings->value(tr("Settings/DBDir"), tr("")).toString();
  if (text.length() > 0)
  {
    ui->editDBDir->setText(text);
  }
  bool checked = pSettings->value(tr("Settings/Compress"), true).toBool();
  ui->checkCompress->setChecked(checked);
  checked = pSettings->value(tr("Settings/SkipDuplicates"), true).toBool();
  ui->checkDuplicates->setChecked(checked);
  text = pSettings->value(tr("Settings/Database"), tr("library1")).toString();
  if (text.length() > 0)
  {
    ui->editDatabase->setText(text);
  }
  text = pSettings->value(tr("Settings/Username"), tr("lib")).toString();
  if (text.length() > 0)
  {
    ui->editUsername->setText(text);
  }
  text = pSettings->value(tr("Settings/Password"), tr("library")).toString();
  if (text.length() > 0)
  {
    ui->editPassword->setText(text);
  }
  sHostName = pSettings->value(tr("Settings/HostName"), tr("localhost")).toString();
}

