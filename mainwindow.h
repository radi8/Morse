#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include "ui_mainwindow.h"


class GenerateMorse;


class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT
public:
	MainWindow();
protected slots:
	void wpmChanged(double);
	void ditFactorChanged(double);
	void dahFactorChanged(double);
	void intraFactorChanged(double);
	void charFactorChanged(double);
	void wordFactorChanged(double);
	void morseGeneratorStart();
	void morseGeneratorStop();
private:
	GenerateMorse *morse;
	void updateWpmLabel();

};


#endif
