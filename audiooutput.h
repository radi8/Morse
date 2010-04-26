#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

/**
 * @file
 * @author Holger Schurig, DH3HS
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

#include <QObject>


class QIODevice;
class QAudioOutput;
class QTimer;
class SineSource;


/*!
 * \brief Sound generator for short beeps using the Qt Multimedia framework
 */
class AudioOutput : public QObject
{
	Q_OBJECT
public:
	AudioOutput(QObject *parent);
	~AudioOutput();
public slots:
	void playSound(unsigned int ms);

private:
	SineSource *gen; //!< \brief QIODevice which generates sound
	char *buffer;    //!< \brief Buffer that holds sound samples

	QAudioOutput *audioOutput; //!< \brief Sound output device from Qt's multimedia
	QIODevice *output;         //!< \brief QIODevice associated to \ref audioOutput
	QTimer *timer;             //!< \brief Timer to call \ref writeMore()

private slots:
	void writeMore();
};


#endif
