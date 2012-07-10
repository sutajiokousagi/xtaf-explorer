#ifndef XTAFDISK_H
#define XTAFDISK_H

#include <QObject>
#include <QFile>

class XtafDisk : public QObject
{
	Q_OBJECT
public:
	explicit XtafDisk(QObject *parent = 0);
	void setFile(QFile &file);
	int read(quint64 offset, char *bytes, quint64 size);
	quint64 size(void);

private:
	QFile myFile;
	
signals:
	
public slots:
	
};

#endif // XTAFDISK_H
