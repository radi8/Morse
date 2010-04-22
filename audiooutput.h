#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H


#include <QObject>


class QIODevice;
class QAudioOutput;
class QTimer;
class SineSource;


class AudioOutput : public QObject
{
	Q_OBJECT
public:
	AudioOutput(QObject *parent);
	~AudioOutput();
public slots:
	void playSound(unsigned int ms);

private:
	SineSource *gen; // Sine-Wave generator
	char *buffer;   // Sine-Wave buffer

	QAudioOutput *audioOutput;
	QIODevice *output;
	QTimer *timer;

private slots:
	void writeMore();
};


#endif
