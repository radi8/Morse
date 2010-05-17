#define DEBUGLVL 3
#include "mydebug.h"


#include "characters.h"
#include <QHeaderView>


const int COL_NO      = 0;
const int COL_SIGN    = 1;
const int COL_CODE    = 2;
const int COL_ENABLED = 3;
const int COL_RIGHT   = 4;
const int COL_WRONG   = 5;
const int COL_PERCENT = 6;
const int COL_LAST    = 6;


QList<MorseCharacter> chars;


CharacterModel::CharacterModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	MYTRACE("CharacterModel::CharacterModel");

	store("a",   ".-");
	store("b",   "-...");
	store("c",   "-.-.");
	store("d",   "-..");
	store("e",   ".");
	store("f",   "..-.");
	store("g",   "--.");
	store("h",   "....");
	store("i",   "..");
	store("j",   ".---");
	store("k",   "-.-");
	store("l",   ".-..");
	store("m",   "--");
	store("n",   "-.");
	store("o",   "---");
	store("p",   ".--.");
	store("q",   "--.-");
	store("r",   ".-.");
	store("s",   "...");
	store("t",   "-");
	store("u",   "..-");
	store("v",   "...-");
	store("w",   ".--");
	store("x",   "-..-");
	store("y",   "-.--");
	store("z",   "--..");
	store("1",   ".----");
	store("2",   "..---");
	store("3",   "...--");
	store("4",   "....-");
	store("5",   ".....");
	store("6",   "-....");
	store("7",   "--...");
	store("8",   "---..");
	store("9",   "----.");
	store("0",   "-----");
	//store("*oA", ".--.-");
	store("ä",   ".-.-");
	store("ö",   "---.");
	store("ü",   "..--");
	store("*",   "--..--");
	store(":",   "---...");
	store(";",   "-.-.-.");
	store("?",   "..--..");
	store("-",   "-....-");
	store("_",   "..--.-");
	store("(",   "-.--.");
	store(")",   "-.--.-");
	store("'",   ".----.");
	store("=",   "-...-");
	store("+",   ".-.-.");
	store("/",   "-..-.");
	store("@",   ".--.-.");
	store("CH",  "----");
	// http://en.wikipedia.org/wiki/Prosigns_for_Morse_code
	store("AR",  ".-.-.");    // end of message
	store("AS",  ".-...");    // wait (followed by seconds)
	store("BK",  "-···-·-");  // Break
	store("BT",  "-..-.");    // separator within message, written as "="
	store("CL",  "-·-··-··"); // going "off-the-air"
	store("CT",  "-·-·-");    // beginning of message, same as KA
	store("DO",  "-··---");   // shift to japanese wabun code
	store("KN",  "-·--·");    // invite names station to send
	store("SK",  "...-.-");   // end of contract
	store("SN",  "...-.");    // understood
	store("SO",  "...---..."); // SOS
	store("VA",  "...-.-");   // same as SK
	store("VE",  "...-.");    // same as SN
	store("KA",  "-.-.-");    // same as CT
	store("TV",  "-..-.");    // same as BT
	store("HH",  "........"); // Error
	store(" ",   " ");

	chars[10].enabled = true;
}


void CharacterModel::store(const QString &sign, const QString &code)
{
	MorseCharacter c;
	foreach(c, chars) {
		if (c.sign == sign) {
			qWarning("'%s' already defined", qPrintable(sign));
			return;
		}
	}
	c.no = chars.count();
	c.sign = sign;
	c.code = code;
	c.right = 0;
	c.wrong = 0;
	c.enabled = false;
	c.lastWrong = false;
	chars.append(c);
}


int CharacterModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return chars.count();
}


int CharacterModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return COL_LAST+1;
}


QVariant CharacterModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		switch (section) {
		case COL_NO:      return QString(tr("Number"));
		case COL_SIGN:    return QString(tr("Sign"));
		case COL_CODE:    return QString(tr("Morse"));
		case COL_ENABLED: return QString(tr("Enabled"));
		case COL_RIGHT:   return QString(tr("Right"));
		case COL_WRONG:   return QString(tr("Wrong"));
		case COL_PERCENT: return QString(tr("Error rate"));
		default:
			return QString("Column %1").arg(section);
		}
	} else
		return QString("Row %1").arg(section);
}


QVariant CharacterModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int row = index.row();

	if (row >= chars.count())
		return QVariant();

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case COL_NO: return chars[row].no;
		case COL_SIGN: return chars[row].sign;
		case COL_CODE: return chars[row].code;
		case COL_ENABLED: return QString("");
		case COL_RIGHT: return chars[row].right;
		case COL_WRONG: return chars[row].wrong;
		case COL_PERCENT: {
			int total = chars[row].right + chars[row].wrong;
			if (total)
				return QString("%1 %").arg(chars[row].wrong*100/total);
			else
				return QString("100 %");
			}
		default:
			return QString("unk %1").arg(index.row());
		}
	} else
	if (index.column() == COL_ENABLED && role == Qt::CheckStateRole) {
		bool checked = chars[row].enabled;
		return checked ? Qt::Checked : Qt::Unchecked;
	}

	return QVariant();
}


Qt::ItemFlags CharacterModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	Qt::ItemFlags flags = QAbstractItemModel::flags(index);

	switch (index.column()) {
	case COL_SIGN:
	case COL_CODE:
	case COL_RIGHT:
	case COL_WRONG:
		return flags | Qt::ItemIsEditable;
	case COL_ENABLED:
		return flags | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
	}
	return Qt::ItemIsEnabled;
}


bool CharacterModel::setData(const QModelIndex &index, const QVariant &value,
             int role)
{
	if (!index.isValid())
		return false;
	int row = index.row();

	if (role == Qt::EditRole) {
		switch (index.column()) {
		case COL_SIGN:
			chars[row].sign = value.toString();
			break;
		case COL_CODE:
			chars[row].code = value.toString();
			break;
		case COL_RIGHT:
			chars[row].right = value.toUInt();
			break;
		case COL_WRONG:
			chars[row].wrong = value.toUInt();
			break;
		default:
			return false;
		}
		emit dataChanged(index, index);
		return true;
	}
	if (index.column() == COL_ENABLED && role == Qt::CheckStateRole) {
		chars[row].enabled = value.toBool();
		emit dataChanged(index, index);
		return true;
	}
	return false;
}


static Qt::SortOrder sortOrder = Qt::AscendingOrder;


static bool lessNo(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %d < %d", sortOrder, one.no, two.no);
	if (sortOrder == Qt::AscendingOrder)
		return one.no < two.no;
	else
		return one.no > two.no;
}


static bool lessSign(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %s < %s", sortOrder, qPrintable(one.sign), qPrintable(two.sign));

	if (sortOrder == Qt::AscendingOrder)
		return one.sign < two.sign;
	else
		return one.sign > two.sign;
}


static bool lessCode(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %s < %s", sortOrder, qPrintable(one.code), qPrintable(two.code));


	QString c1;
	foreach(QChar c, one.code) {
		if (c == '.')
			c1.append("b");
		else
		if (c == '-')
			c1.append("c");
		else
		if (c == ' ')
			c1.append("x");
	}
	while (c1.count() < 10)
		c1.append("a");
	QString c2;
	foreach(QChar c, two.code) {
		if (c == '.')
			c2.append("b");
		else
		if (c == '-')
			c2.append("c");
		if (c == ' ')
			c2.append("x");
	}
	while (c2.count() < 10)
		c2.append("a");

	if (sortOrder == Qt::AscendingOrder)
		return c1 < c2;
	else
		return c1 > c2;
}


static bool lessEnabled(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %d < %d", sortOrder, one.enabled, two.enabled);

	if (sortOrder == Qt::AscendingOrder)
		return one.enabled > two.enabled;
	else
		return one.enabled < two.enabled;
}


static bool lessRight(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %d < %d", sortOrder, one.right, two.right);

	if (sortOrder == Qt::AscendingOrder)
		return one.right > two.right;
	else
		return one.right < two.right;
}


static bool lessWrong(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %d < %d", sortOrder, one.wrong, two.wrong);

	if (sortOrder == Qt::AscendingOrder)
		return one.wrong < two.wrong;
	else
		return one.wrong > two.wrong;
}


static bool lessPercent(const MorseCharacter &one, const MorseCharacter &two)
{
	//MYTRACE("%d sort %d < %d", sortOrder, one.percent, two.percent);

	int t1 = one.right + one.wrong;
	int t2 = two.right + two.wrong;
	int p1 = 100;
	int p2 = 100;
	if (t1)
		p1 = one.wrong*100/t1;
	if (t2)
		p2 = two.wrong*100/t2;

	if (sortOrder == Qt::AscendingOrder)
		return p1 < p2;
	else
		return p1 > p2;
}

void CharacterModel::sort(int column, Qt::SortOrder order)
{
	MYVERBOSE("CharacterModel::sort(%d, %d)", column, order);

	sortOrder = order;

	switch (column) {
	case COL_NO:      qStableSort(chars.begin(), chars.end(), lessNo); break;
	case COL_SIGN:    qStableSort(chars.begin(), chars.end(), lessSign); break;
	case COL_CODE:    qSort(chars.begin(), chars.end(), lessCode); break;
	case COL_ENABLED: qStableSort(chars.begin(), chars.end(), lessEnabled); break;
	case COL_RIGHT:   qStableSort(chars.begin(), chars.end(), lessRight); break;
	case COL_WRONG:   qStableSort(chars.begin(), chars.end(), lessWrong); break;
	case COL_PERCENT: qStableSort(chars.begin(), chars.end(), lessPercent); break;
	}
	emit dataChanged(index(0, 0), index(chars.count(), COL_LAST));
}


CharacterView::CharacterView(QWidget *parent)
	: QTableView(parent)
{
	MYTRACE("CharacterView::CharacterView");
	setAlternatingRowColors(true);
	verticalHeader()->hide();
	horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	setTabKeyNavigation(false);
	setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	connect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
	        SLOT(sortByColumn(int)) );
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
}
