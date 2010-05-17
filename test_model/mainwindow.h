#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include "ui_mainwindow.h"


class CharacterModel;


class MainWindow : public QMainWindow, Ui::MainWindow
{
	Q_OBJECT
public:
	MainWindow();
private:
	CharacterModel *model;
};


#endif
