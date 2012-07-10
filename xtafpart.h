#ifndef XTAFPART_H
#define XTAFPART_H

#include <QObject>
#include "xtafdisk.h"


class XtafPart : public QObject
{
	Q_OBJECT
public:    
    enum Format {
        Invalid,
        None,
        SFCX,
        XTAF,
    };
    explicit XtafPart(QObject *parent = 0);
    unsigned int count(void);	// Returns the number of partitions
    const char *name(quint8 partition);
    const char *name();
    enum Format format(quint8 partition);
    enum Format format(void);
    int setPartition(quint8 partition);
    int setDisk(XtafDisk *disk);


    qint64 length(void);
    int read(quint64 offset, void *bytes, quint64 size);

private:
    int currentPartition;
    XtafDisk *disk;

signals:
	
public slots:
	
};

#endif // XTAFPART_H
