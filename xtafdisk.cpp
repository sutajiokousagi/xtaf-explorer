#include <QDebug>
#include "xtafdisk.h"

XtafDisk::XtafDisk(QObject *parent) :
    QObject(parent)
{

}

void XtafDisk::setFile(QFile &file)
{
	myFile.setFileName(file.fileName());
	myFile.open(QIODevice::ReadOnly);
}

int XtafDisk::read(quint64 offset, char *bytes, quint64 size)
{
	qint64 bytesRead;
    qint64 bytesTotal = 0;
    myFile.seek(offset);
    while (size) {
		bytesRead = myFile.read(bytes, size);
		if (-1 == bytesRead)
			return -1;
		bytesTotal += bytesRead;
		bytes += bytesRead;
		size -= bytesRead;
	}
	return bytesRead;
}

quint64 XtafDisk::size(void)
{
	return myFile.size();
}
