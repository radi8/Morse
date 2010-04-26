#ifndef MORSE_SCROLLER_H
#define MORSE_SCROLLER_H

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

#include <QLabel>
#include <QList>

class QTimer;



/*!
 * \brief Visual representation for morse code
 */
class MorseScroller : public QLabel {
	Q_OBJECT
public:
	MorseScroller(QWidget *parent=0, Qt::WindowFlags f=0);
	virtual void paintEvent(QPaintEvent *event);
public slots:
	void setSound(bool);
private slots:
	void slotScroll();
private:
	QTimer *timer;    //!< \brief Timer to periodically call \ref slotScroll()
	QList<bool> data; //!< \brief List of sound states, to draw the dots and dahs
	bool on;          //!< \brief Current sound state, set by \ref setSound()
};

#endif
