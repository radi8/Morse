#include <QAbstractTableModel>
#include <QTableView>
#include <QList>


class MorseCharacter {
public:
	quint32 no;
	QString sign;
	QString code;
	quint32 right;
	quint32 wrong;
	bool enabled;
	bool lastWrong;

	//bool operator<(const MorseCharacter &other) const;
};


extern QList<MorseCharacter> chars;


class CharacterModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	CharacterModel(QObject *parent=0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex() ) const;

	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
	                    int role = Qt::DisplayRole) const;

	virtual void sort(int column, Qt::SortOrder order=Qt::AscendingOrder);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value,
	             int role = Qt::EditRole);
private:
	void store(const QString &sign, const QString &code);
};




class CharacterView : public QTableView {
	Q_OBJECT
public:
	CharacterView(QWidget *parent=0);

	//void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags);
};
