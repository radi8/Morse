#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include "ui_mainwindow.h"


class TeachMorse;
class GenerateMorse;
class AudioOutput;


class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT
public:
	MainWindow();
private:
	TeachMorse *teach;
	GenerateMorse *morse;
	AudioOutput *audio;
private slots:
	void slotCheck();
	void slotGenerate();
};


#endif
