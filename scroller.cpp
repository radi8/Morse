#define DEBUGLVL 1
#include "mydebug.h"

/**
 * @file
 * @author Holger Schurig, DH3HS
 *
 * @section DESCRIPTION
 *
 * Visual representation for morse code.
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

#include <QTimer>
#include <QPainter>

#include "scroller.h"


/*!
 * \brief Visual representation for morse code
 *
 * This widget creates a horizontal display element, where morse code can be
 * seen as dots and dashes. They move automatically from right to left.
 *
 * @param parent  \c QWidget parent, if any
 * @param f       \c Qt::WindowFlags, if any
 *
 * Usage:
 *
 * \code
 *    MorseScroller *scroll = new MorseScroller(0);
 *    scroll->show();
 *    scroll->resize(400, 15);
 *    connect(morse, SIGNAL(playSound(bool)), scrollWidget, SLOT(setSound(bool)) );
 *    morse->append(...)
 *    morse->play()
 * \endcode
 */
MorseScroller::MorseScroller(QWidget *parent, Qt::WindowFlags f)
	: QLabel(parent, f)
	, on(false)
{
	MYTRACE("MorseScroller::MorseScroller");

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(slotScroll()) );
	timer->start(50);
}


/*!
 * \brief Used to signal if currently a sound is audible
 *
 * @param onoff   specifies if currently sound is audible or not
 */
void MorseScroller::setSound(bool onoff)
{
	MYTRACE("MorseScroller::slotSound(%d)", onoff);
	on = onoff;
}


/**
 * \brief Controls the animation
 *
 * This slot get's called periodically from \ref timer. It moves the current
 * sound state (from \ref setSound()) into \ref data and asks the GUI to
 * update itself. This in turn calls \ref paintEvent().
 */
void MorseScroller::slotScroll()
{
	MYTRACE("MorseScroller::slotScroll, on %d", on);
	data.append(on);
	update();
}


/**
 * \brief GUI update method
 *
 * This iterates over \ref data and draws it's contents from right to left into
 * the visible GUI space.
 *
 * Any \ref data elements in excess will then be deleted. This prevents \ref
 * data from overflowing.
 */
void MorseScroller::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);
	MYTRACE("MorseScroller::paintEvent");

	QPainter paint(this);
	int x = width()+1;
	QPen penOff(Qt::gray, 3);
	QPen penOn(Qt::black, 3);
	QList<bool>::const_iterator i;
	i = data.constEnd();
	i--;
	while (x > 0 && i != data.constBegin()) {
		paint.setPen(*i ? penOn : penOff);
		paint.drawPoint(x, 7);
		x -= 3;
		i--;
	}
	while (i != data.constBegin())
		data.removeFirst();
}
