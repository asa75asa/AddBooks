#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QThread>
class QSettings;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSettings *pSettings;
    QString sHostName;
    bool bStarted;
    MyThread thread;
    void saveSettings();
    void loadSettings();
private slots:
    void on_buttonDBDir_clicked();
    void on_buttonNewDir_clicked();
    void on_buttonStart_clicked();
};



#endif // MAINWINDOW_H
