#define DEBUGLVL 1
#include "mydebug.h"

#include <QTimer>
#include <QPainter>

#include "scroller.h"

const int max_width = 640;

MorseScroller::MorseScroller(QWidget *parent, Qt::WindowFlags f)
	: QLabel(parent, f)
	, on(false)
{
	MYTRACE("MorseScroller::MorseScroller");

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(slotScroll()) );
	timer->start(50);

	//setMaximumWidth(max_width);
}


void MorseScroller::setSound(bool onoff)
{
	MYTRACE("MorseScroller::slotSound(%d)", onoff);
	on = onoff;
}


void MorseScroller::slotScroll()
{
	MYTRACE("MorseScroller::slotScroll, on %d", on);
	data.append(on);
	update();
}


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
	while (x > 0 && i != data.constBegin()) {
		paint.setPen(*i ? penOn : penOff);
		paint.drawPoint(x, 7);
		x -= 3;
		i--;
	}
	while (i != data.constBegin())
		data.removeFirst();
}
