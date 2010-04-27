#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include "ui_mainwindow.h"


class TeachMorse;
class GenerateMorse;


class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT
public:
	MainWindow();
private:
	TeachMorse *teach;
	GenerateMorse *morse;
private slots:
	void slotCheck();
};


#endif
