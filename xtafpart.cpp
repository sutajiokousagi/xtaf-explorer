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
static struct partition_def partitions_rom[] = {
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
		{
				NULL,
				0,
				0,
				XtafPart::None,
		}, // Sentinal
};

static struct partition_def partitions_module[] = {
	{
			"Online",
			0,
			0x8000000ULL,
			XtafPart::XTAF,
	},
	{
			"Cache",
			0x8000000ULL,
			0x4000000ULL,
			XtafPart::XTAF,
	},
	{
			"Empty",
			0xc000000ULL,
			0x8000000ULL,
			XtafPart::XTAF,
	},
	{
			"Dashboard",
			0x14000000ULL,
			0xce30000ULL,
			XtafPart::XTAF,
	},
	{
			"Data",
			0x20e30000ULL,
			0,
			XtafPart::XTAF,
	},
	{
			NULL,
			0,
			0,
			XtafPart::None,
	}, // Sentinal
};

static struct partition_def *partitions = partitions_rom;

XtafPart::XtafPart(QObject *parent) :
    QObject(parent),
	_currentPart(-1)
{
}


unsigned int XtafPart::count(void)
{
	int count;
	for (count=0; partitions[count].name != NULL; count++) {
		;
	}
	return count;
}

const char *XtafPart::name(quint8 part_id)
{
    return part_id < count() ? partitions[part_id].name : NULL;
}

const char *XtafPart::name(void)
{
	return partitions[_currentPart].name;
}

enum XtafPart::Format XtafPart::format(quint8 part_id)
{
    if (part_id >= count())
        return XtafPart::None;
    return partitions[part_id].format;
}

int XtafPart::currentPartition()
{
	return _currentPart;
}

enum XtafPart::Format XtafPart::format(void)
{
	if (_currentPart >= (int)count() || _currentPart < 0)
        return XtafPart::Invalid;
	return partitions[_currentPart].format;
}


int XtafPart::setPartition(quint8 part_id)
{
    if (part_id >= count())
        return -1;
	_currentPart = part_id;
        return 0;
}

int XtafPart::setDisk(XtafDisk *newDisk)
{
	char magic[4];
    if (!newDisk)
        return -1;
    disk = newDisk;

	disk->read(0, magic, sizeof(magic));
	if (!memcmp(magic, "XTAF", sizeof(magic))) {
		partitions = partitions_module;
	}
	else
		partitions = partitions_rom;

    return 0;
}


qint64 XtafPart::length(void) {
    qint64 length;

	if (!disk || _currentPart < 0)
        return -1;

	length = partitions[_currentPart].length;

    if (!length)
		length = disk->size() - partitions[_currentPart].offset;
    return length;
}

quint64 XtafPart::start(void)
{
	if (!disk || _currentPart < 0)
		return -1;
	return partitions[_currentPart].offset;
}

int XtafPart::read(quint64 offset, void *bytes, quint64 size)
{
	if (!disk || _currentPart < 0)
        return -1;

    if (offset > (quint64)length())
        return -1;
    if (offset+size > (quint64)length())
        size = length()-offset;

	offset += partitions[_currentPart].offset;
    return disk->read(offset, (char *)bytes, size);
}
