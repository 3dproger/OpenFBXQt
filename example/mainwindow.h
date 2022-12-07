#pragma once

#include "openfbxqt.h"
#include <QMainWindow>

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
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionExit_triggered();

private:
    void open(const QString& fileName);
    void addLogMessage(const ofbxqt::Note& note);

    Ui::MainWindow *ui;
};
