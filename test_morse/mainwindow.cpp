#define DEBUGLVL 0
#include "mydebug.h"

#include "mainwindow.h"
#include "morse.h"
#include "scroller.h"
#include "audiooutput.h"

#include <QTimer>


MainWindow::MainWindow()
{
	MYTRACE("MainWindow::MainWindow");
	setupUi(this);

	morse = new GenerateMorse(this);
	connect(loopCheckBox, SIGNAL(toggled(bool)), morse, SLOT(setLoop(bool)) );
	connect(morse, SIGNAL(charChanged(const QString &)),
	        morseChar, SLOT(setText(const QString &)) );
	connect(morse, SIGNAL(symbolChanged(const QString &)),
	        morseSymbol, SLOT(setText(const QString &)) );
	connect(morse, SIGNAL(hasStopped()), morseChar, SLOT(clear()) );
	connect(morse, SIGNAL(hasStopped()), morseSymbol, SLOT(clear()) );
	wpmSpinBox->setValue(10);
	updateWpmLabel();

	connect(morse, SIGNAL(hasStopped()), this, SLOT(morseGeneratorStop()) );
	morse->append("by dh3hs");
	//morse->append("paris paris paris paris paris ");
	QTimer::singleShot(1, this, SLOT(morseGeneratorStart()) );

	AudioOutput *audio = new AudioOutput(this);
	connect(morse, SIGNAL(playSound(unsigned int)), audio, SLOT(playSound(unsigned int)) );
	//connect(morse, SIGNAL(hasStopped()), qApp, SLOT(quit()) );

	connect(morse, SIGNAL(playSound(bool)), scrollWidget, SLOT(setSound(bool)) );

	connect(morse, SIGNAL(maxElements(int)), progressBar, SLOT(setMaximum(int)) );
	connect(morse, SIGNAL(currElement(int)), progressBar, SLOT(setValue(int)) );
}


void MainWindow::wpmChanged(double wpm)
{
	morse->setWpm(wpm);
	updateWpmLabel();
}

void MainWindow::ditFactorChanged(double f)
{
	morse->setDitFactor(f);
	updateWpmLabel();
}

void MainWindow::dahFactorChanged(double f)
{
	morse->setDahFactor(f);
	updateWpmLabel();
}

void MainWindow::intraFactorChanged(double f)
{
	morse->setIntraFactor(f);
	updateWpmLabel();
}

void MainWindow::charFactorChanged(double f)
{
	morse->setCharFactor(f);
	updateWpmLabel();
}

void MainWindow::wordFactorChanged(double f)
{
	morse->setWordFactor(f);
	updateWpmLabel();
}


void MainWindow::updateWpmLabel()
{
	wpmLabel->setText( QString().setNum(morse->getWpm(), 'g', 4));
}


void MainWindow::morseGeneratorStart()
{
	morse->play();
	morseStartButton->setEnabled(false);
	morseStopButton->setEnabled(true);
}


void MainWindow::morseGeneratorStop()
{
	morse->stop();
	morseStartButton->setEnabled(true);
	morseStopButton->setEnabled(false);
}
