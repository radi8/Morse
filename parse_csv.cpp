#include "parse_csv.h"

#include <QFile>

bool ParseCSV::parse()
{
	QFile f(fname);
	if (! f.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QByteArray data;
	const int maxSize = 4096;
	enum {
		stBegin,
		stInString,
		stInEscape,
		stInNum,
	} state;
	state = stBegin;
	QString item;
	int field = 0;
	int recNo = 0;
	while (1) {
		data = f.read(maxSize);
		if (data.isEmpty())
			break;

		foreach(char c, data) {
			//qDebug("state %d, '%c'", state, c);
			if (state == stBegin) {
				if (c == '"') {
					// Start of string
					state = stInString;
					continue;
				} else
				if (c == ',') {
					// Field delimter
					field++;
					continue;
				} else
				if (c >= '0' && c <= '9') {
					// Start of Number
					item = c;
					state = stInNum;
					continue;
				}
				if (c == ' ' || c == '\t') {
					continue;
				}
			} else
			if (state == stInString) {
				if (c == '"') {
					// End of string
					setData(field, item);
					item.clear();
					state = stBegin;
					continue;
				} else
				if (c == '\\') {
					// Escaped character inside string
					state = stInEscape;
					continue;
				} else {
					item.append(c);
					continue;
				}
			} else
			if (state == stInEscape) {
				// Character after escape character
				item.append(c);
				state = stInString;
				continue;
			}
			if (state == stInNum) {
				if ((c >= '0' && c <= '9') || c == '.') {
					// digit for number
					item.append(c);
					continue;
				} else
				// if (c == ' ' || c == '\t') {
				// 	continue;
				// } else
				if (c == ',' || c == '\r' || c == '\n') {
					// end of number
					setData(field, item);
					item.clear();
					field++;
					state = stBegin;
					if (c != ',') {
						// end of line
						saveRecord();
						field = 0;
						recNo++;
					}
					continue;
				}
			}
			qWarning("unhandled: state %d, char '%c'", state, c);
			return false;
		}
	}
	f.close();
	return true;
}


#if 0
void ParseCSV::setData(int field, const QString &item)
{
	qDebug("set %d to \"%s\"", field, qPrintable(item));
}


void ParseCSV::saveRecord()
{
	qDebug("next");
}
#endif
