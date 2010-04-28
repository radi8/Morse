#define DEBUGLVL 0
#include "mydebug.h"

/**
 * @file
 * @author Holger Schurig, DH3HS
 *
 * @section DESCRIPTION
 *
 * Simple sine audio generator.
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QTimer>

#include <math.h>
#include "audiooutput.h"

#ifndef M_PI
/*!
 * \brief Representation of pi
 */
#define M_PI 3.14159265358979323846
#endif

/*!
 * \brief Sample rate of sound card
 *
 * In Hertz.
 */
#define SAMPLE_RATE 44100

/*!
 * \brief Buffer size for \ref AudioOutput
 *
 * In Bytes.
 */
#define BUFFER_SIZE 8196


/*!
 * \brief Sine wave generator source for \ref AudioOutput
 */
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
	int *buffer;  //!< \brief Sine wave buffer
	int *sendpos; //!< \brief Current pos into the circular \ref buffer
	int *end;     //!< \brief Last position in \ref buffer, for faster comparison
	int samples;  //!< \brief Samples to play for desired sound duration
};


/*!
 * \brief Sine wave generator source for \ref AudioOutput
 *
 * @param freq    desired frequency of sine wave
 * @param parent  parent QObject, if any
 *
 * This class is taylored to be used with \ref AudioOutput, but basically
 * it's usage pattern looks like this:
 *
 * \code
 *   gen = new SineSource(800, this);
 *   gen->setDuration(ms);
 *   while (1) {
 *        int l = gen->read(buffer, audioOutput->periodSize());
 *        if (!l) break;
 *        	output->write(buffer, l)
 *   }
 * \endcode
 */
SineSource::SineSource(int freq, QObject *parent)
	: QIODevice(parent)
	, buffer(0)
{
	setFreq(freq);
	open(QIODevice::ReadOnly);
}


/*!
 * \brief Class destructor
 *
 * Simply get's rid of \ref buffer.
 */
SineSource::~SineSource()
{
	delete[] buffer;
}


/*!
 * \brief Change generated frequency
 *
 * Calling this method creates a new \ref buffer with 3 full waves of the
 * desired frequency. After this, you can call \ref setDuration() and than
 * consume the bytes using \ref readData().
 *
 * @param frequency desiged frequency in Hz. Should be below 10000 Hertz.
 */
void SineSource::setFreq(int frequency)
{
	MYTRACE("SineSource::setFreq(%d)", frequency);

	if (buffer)
		delete[] buffer;
	freq = frequency;

	const int upper_freq = 10000;
	const int full_waves = 3;

	// Arbitrary upper frequency
	if (freq > upper_freq)
		freq = upper_freq;

	// We create a buffer with some full waves of freq,
	// therefore we need room for this many samples:
	int buflen = SAMPLE_RATE * full_waves / freq;

	MYVERBOSE("buf needs to hold %d samples", buflen);

	buffer = new int[buflen];

	// Now fill this buffer with the sine wave
	int *t = buffer;
	for (int i = 0; i < buflen; i++) {
		int value = 32767.0 * sin(M_PI * 2 * i * freq / SAMPLE_RATE);
		MYVERBOSE("%4d: %6d, pos %d", i, value, t - buffer);
		*t++ = value;
	}

	sendpos = buffer;
	end = buffer + buflen;
	samples = 0;
}


/*!
 * \brief Generate sine for \c ms milliseconds
 *
 * This calculates how many \ref samples from \ref buffer are needed for the
 * specified sound duration duration. Later, \ref readData() won't return
 * more than this number of samples.
 *
 * @param ms   sound duration in milliseconds
 */
void SineSource::setDuration(int ms)
{
	samples = (SAMPLE_RATE * ms) / 1000;
	samples &= 0x7ffffffe;
	sendpos = buffer;
}


/*!
 * \brief Returns entries from \ref buffer
 *
 * Is is a on overwritten method from \c QIODevice which will return the
 * samples of a sine-wave.
 *
 * You need to call \ref setDuration() first
 *
 * @param data    destination address
 * @param maxlen  maximum bytes to copy
 * @returns       number of bytes copied
 */
qint64 SineSource::readData(char *data, qint64 maxlen)
{
	MYTRACE("SineSource::readData(data, %lld, samples %d)", maxlen, samples);

	quint64 len = maxlen;

	char *t = data;
	while (len) {
		// As long as we should provide samples, do this:
		if (samples) {
			int value = *sendpos++;
			//TODO: this is the place where we could modify the
			//value, e.g. to ramp it up or down, or to attenuate it
			if (sendpos == end)
				sendpos = buffer;
			*t++ = value        & 0xff;
			*t++ = (value >> 8) & 0xff;
			samples--;
		} else {
			// But afterwards, return zero
			*t++ = 0;
			*t++ = 0;
		}
		len -= 2;
	}
	return maxlen;
}


/*!
 * \brief Dummy implementation
 *
 * This dummy implementation does nothing and is just needed to inherit
 * successfully from \c QIODevice.
 */
qint64 SineSource::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data);
	Q_UNUSED(len);

	return 0;
}




/*!
 * \brief Audio generator for morse code
 *
 * This class generates sound for some milliseconds.
 *
 * @param parent  QObject parent, if any
 *
 * Usage:
 * \code
 *   AudioOutput *audio = new AudioOutput(this);
 *   connect(morse, SIGNAL(playSound(int)), audio, SLOT(playSound(int)) );
 *   morese->append(...);
 *   morse->play();
 * \endcode
 */
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


/*!
 * \brief Class destructor
 *
 * Destroy's \ref buffer.
 */
AudioOutput::~AudioOutput()
{
	MYTRACE("AudioOutput::~AudioOutput");

	delete[] buffer;
}


/*!
 * \brief Send more data from \ref SineSource to \c QAudioOutput
 *
 * This is an interal signal, called via \ref timer. This timer
 * is called periodically as long as more data should be played.
 */
void AudioOutput::writeMore()
{
	MYTRACE("AudioOutput::writeMore");

	if (!audioOutput)
		return;

	if (audioOutput->state() == QAudio::StoppedState)
		return;

	// If we would write this into a file, we could convert it to a WAV
	// file with this command:
	//       sox -r 44100 -e signed -b 16 -c 1 a.raw a.wav
	int chunks = audioOutput->bytesFree() / audioOutput->periodSize();
	while (chunks) {
		int l = gen->read(buffer, audioOutput->periodSize());
		if (l > 0)
			output->write(buffer, l);
		chunks--;
	}
	timer->start(30);
}


/*!
 * \brief Start sound generation
 *
 * @param ms  Desired sound duration in milliseconds.
 */
void AudioOutput::playSound(unsigned int ms)
{
	MYTRACE("AudioOutput::playSound(%d)", ms);

	gen->setDuration(ms);
	audioOutput->resume();
	writeMore();
}
