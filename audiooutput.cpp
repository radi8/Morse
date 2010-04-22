#define DEBUGLVL 0
#include "mydebug.h"

#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QTimer>

#include <math.h>
#include "audiooutput.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 8196


class SineSource : public QIODevice
{
public:
	SineSource(int freq, QObject *parent);
	~SineSource();
	void setFreq(int freq);
	void setDuration(int ms);

	qint64 readData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);

private:
	int freq;
	unsigned char *buffer;  // Sine wave buffer
	unsigned char *sendpos; // current pos
	unsigned char *end;     // for faster comparison
	int samples;         // samples to play for desired sound duration
};


SineSource::SineSource(int _freq, QObject *parent)
	: QIODevice(parent)
	, buffer(0)
{
	setFreq(_freq);
	open(QIODevice::ReadOnly);
}


SineSource::~SineSource()
{
	delete[] buffer;
}


void SineSource::setFreq(int _freq)
{
	MYTRACE("SineSource::setFreq(%d)", freq);

	if (buffer)
		delete[] buffer;
	freq = _freq;

	const int upper_freq = 10000;
	const int full_waves = 3;

	// Arbitrary upper frequency
	if (freq > upper_freq)
		freq = upper_freq;

	// We create a buffer with some full waves of freq,
	// therefore we need room for this many samples:
	int buflen = SAMPLE_RATE * full_waves / freq;

	// as we store two bytes per value, we need to double that
	buflen *= 2;
	MYVERBOSE("calculated buflen: %d", buflen);

	buffer = new unsigned char[buflen];

	// Now fill this buffer with the sine wave
	unsigned char *t = buffer;
	for (int i = 0; i < buflen / 2; i++) {
		int value = 32767.0 * sin(M_PI * 2 * i * freq / SAMPLE_RATE);
		*t++ = value        & 0xff;
		*t++ = (value >> 8) & 0xff;
		//MYVERBOSE("%4d: %6d, pos %d", i, value, t - buffer);
	}

	sendpos = buffer;
	end = buffer + buflen;
}


void SineSource::setDuration(int ms)
{
	samples = (SAMPLE_RATE * ms) / 1000;
	samples &= 0x7ffffffe;
}


qint64 SineSource::readData(char *data, qint64 maxlen)
{
	MYTRACE("SineSource::readData(data, %lld, samples %d)", maxlen, samples);

	if (maxlen > samples)
		maxlen = samples;
	if (!maxlen)
		return 0;

	quint64 len = maxlen;
	while (len--) {
		*data++ = *sendpos++;
		if (sendpos == end)
			sendpos = buffer;
	}
	samples -= maxlen;
	return maxlen;
}


qint64 SineSource::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data);
	Q_UNUSED(len);

	return 0;
}




AudioOutput::AudioOutput(QObject *parent)
	: QObject(parent)
	, audioOutput(0)
{
	buffer = new char[BUFFER_SIZE];

	QAudioFormat settings;
	settings.setFrequency(SAMPLE_RATE);
	settings.setChannels(1);
	settings.setSampleSize(16);
	settings.setCodec("audio/pcm");
	settings.setByteOrder(QAudioFormat::LittleEndian);
	settings.setSampleType(QAudioFormat::SignedInt);

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (!info.isFormatSupported(settings)) {
		qWarning("Default audio format not supported, sound may be distorted");
		settings = info.nearestFormat(settings);
	}

	if (settings.sampleSize() != 16) {
		qWarning("Audio doesn't support 16 bit samples");
		return;
	}

	gen = new SineSource(800, this);

	audioOutput = new QAudioOutput(settings, this);

	output = audioOutput->start();
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(writeMore()));
	timer->setSingleShot(true);
}


AudioOutput::~AudioOutput()
{
	MYTRACE("AudioOutput::~AudioOutput");

	delete[] buffer;
}


void AudioOutput::writeMore()
{
	MYTRACE("AudioOutput::writeMore");

	if (!audioOutput)
		return;

	if (audioOutput->state() == QAudio::StoppedState)
		return;

	int chunks = audioOutput->bytesFree() / audioOutput->periodSize();
	while (chunks) {
		int l = gen->read(buffer, audioOutput->periodSize());
		if (l == 0) {
			audioOutput->suspend();
			return;
		}
		if (l > 0)
			output->write(buffer,l);
		if (l != audioOutput->periodSize())
			break;
		chunks--;
	}
	timer->start(20);
}


void AudioOutput::playSound(unsigned int ms)
{
	MYTRACE("AudioOutput::playSound(%d)", ms);

	gen->setDuration(ms);
	audioOutput->resume();
	writeMore();
}
