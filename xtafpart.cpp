#include <QDebug>
#include "xtafpart.h"


struct partition_def {
        const char              *name;
        quint64                 offset;
        quint64                 length;
        enum XtafPart::Format   format;
};


struct part {
        quint32                 id;
        struct partition_def    *def;
        quint8                  *data;
        struct disk             *disk;
};


/* Hardcoded into the Xbox 360 Phat's ROM */
static struct partition_def partitions[] = {
        {
                "Security Sector",
                0x2000ULL,
                0x40ULL,
                XtafPart::None,
        },
        {
                "System Cache",
                0x80000ULL,
                0x80000000ULL,
                XtafPart::SFCX,
        },
        {
                "Game Cache",
                0x80080000ULL,
                0xA0E30000ULL,
                XtafPart::SFCX,
        },
        {
                "SysExt",
                0x10C080000ULL,
                0xCE30000ULL,
                XtafPart::XTAF,
        },
        {
                "SysExt2",
                0x118EB0000ULL,
                0xCE30000ULL,
                XtafPart::XTAF,
        },
        {
                "Xbox 1",
                0x120eb0000ULL,
                0x10000000ULL,
                XtafPart::XTAF,
        },

        // "Data" partition takes up the rest of the disk
        {
                "Data",
                0x130eb0000ULL,
                0,
                XtafPart::XTAF,
        },
};

XtafPart::XtafPart(QObject *parent) :
    QObject(parent),
    currentPartition(-1)
{
}


unsigned int XtafPart::count(void)
{
    return sizeof(partitions)/sizeof(*partitions);
}

const char *XtafPart::name(quint8 part_id)
{
    return part_id < count() ? partitions[part_id].name : NULL;
}

const char *XtafPart::name(void)
{
    return partitions[currentPartition].name;
}

enum XtafPart::Format XtafPart::format(quint8 part_id)
{
    if (part_id >= count())
        return XtafPart::None;
    return partitions[part_id].format;
}



enum XtafPart::Format XtafPart::format(void)
{
    if (currentPartition >= (int)count() || currentPartition < 0)
        return XtafPart::Invalid;
    return partitions[currentPartition].format;
}


int XtafPart::setPartition(quint8 part_id)
{
    if (part_id >= count())
        return -1;
    currentPartition = part_id;
        return 0;
}

int XtafPart::setDisk(XtafDisk *newDisk)
{
    if (!newDisk)
        return -1;
    disk = newDisk;
    return 0;
}


qint64 XtafPart::length(void) {
    qint64 length;

    if (!disk || currentPartition < 0)
        return -1;

    length = partitions[currentPartition].length;

    if (!length)
        length = disk->size() - partitions[currentPartition].offset;
    return length;
}

int XtafPart::read(quint64 offset, void *bytes, quint64 size)
{
    if (!disk || currentPartition < 0)
        return -1;

    if (offset > (quint64)length())
        return -1;
    if (offset+size > (quint64)length())
        size = length()-offset;

    offset += partitions[currentPartition].offset;
    return disk->read(offset, (char *)bytes, size);
}
