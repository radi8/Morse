#ifndef PARSE_CSV_H
#define PARSE_CSV_H


#include <QString>


class ParseCSV {
public:
	ParseCSV(const QString &name) : fname(name) {};
	bool parse();
	virtual void setData(int field, const QString &item) = 0;
	virtual void saveRecord() = 0;

	QString fname;
};

#endif
