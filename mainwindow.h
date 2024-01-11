#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ltglabel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_readButton_clicked();

    void on_saveButton_clicked();

private:
    Ui::MainWindow *ui;

    LtgLabel m_lbl;
    bool m_measured;

protected:
    void paintEvent(QPaintEvent *e);
};
#endif // MAINWINDOW_H
