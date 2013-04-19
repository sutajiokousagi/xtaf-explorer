#include <QDebug>
#include <QtEndian>
#include "xtaffsys.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>


struct xtaf_dir_entry;
struct xtaf_dir;


struct xtaf {
    uint8_t *chainmap;
    uint64_t length;

    uint8_t  magic[4];
    uint32_t id;
    uint32_t spc; // Sectors per cluster
    uint32_t rdc; // Root directory cluster
    uint32_t cluster_count;
    uint32_t cluster_size;
    uint32_t entry_size;
    uint32_t chainmap_size;

    uint8_t *page_cache; // Temporary cache for reading clusters.
    struct xtaf_dir *current_dir;
};

struct xtaf_dir {
    struct xtaf *xtaf;
    struct xtaf_dir_entry *entries;
    uint32_t entry_count;
    uint32_t cluster;   // Starting cluster
    uint32_t parent;
};

struct xtaf_dir_entry {
    uint8_t  name_len;
    uint8_t  file_flags;
    uint8_t  filename[0x2a];
    uint32_t start_cluster;
    uint32_t file_size;
    uint16_t create_date;
    uint16_t create_time;
    uint16_t access_date;
    uint16_t access_time;
    uint16_t update_date;
    uint16_t update_time;
} __attribute__ ((__packed__));




QString xtaf_datetime_str(uint16_t d, uint16_t t) {
    static char str[64];
    uint32_t year = ((d>>9)&0x7f)+1980;
    uint8_t month = (d>>5)&0x0f;
    uint8_t day = (d>>0)&0x1f;
    uint8_t hours = (t>>11)&0x1f;
    uint8_t minutes = (t>>5)&0x3f;
    uint8_t seconds = 2*((t>>0)&0x1f);
    snprintf(str, sizeof(str)-1, "%04u%02u%02u%02u%02u%02u", year, month, day, hours, minutes, seconds);
    return QString(str);
}



XtafFile::XtafFile(XtafFile *parent, const struct xtaf_dir_entry *entry)
{
    unsigned int nameLength = entry->name_len;
    QChar tempName[sizeof(entry->filename)+1];
    this->_parent = parent;
    quint16 dateInt;
    quint16 timeInt;

    // Swap all relevent entries
    dateInt = qFromBigEndian(entry->access_date);
    timeInt = qFromBigEndian(entry->access_time);
    _accessed = QDateTime::fromString(xtaf_datetime_str(dateInt, timeInt), "yyyyMMddhhmmss");

    dateInt = qFromBigEndian(entry->update_date);
    timeInt = qFromBigEndian(entry->update_time);
    _updated = QDateTime::fromString(xtaf_datetime_str(dateInt, timeInt), "yyyyMMddhhmmss");

    dateInt = qFromBigEndian(entry->create_date);
    timeInt = qFromBigEndian(entry->create_time);
    _created = QDateTime::fromString(xtaf_datetime_str(dateInt, timeInt), "yyyyMMddhhmmss");

    fileSize   = qFromBigEndian(entry->file_size);
    _startCluster = qFromBigEndian(entry->start_cluster);

    unsigned int j;
    // Replace 0xff with 0x00, which shows up sometimes in filenames
    for(j=0; j<sizeof(entry->filename); j++)
        if (entry->filename[j] == 0xff)
            tempName[j] = 0x00;
        else
            tempName[j] = entry->filename[j];

    // Indicates file was deleted
    if (nameLength == 0xe5) {
        deleted = true;
        _name = QString((QChar *)tempName);
    }
    else {
        deleted = false;
        if (nameLength > sizeof(entry->name_len)-1)
            nameLength = entry->name_len;
        tempName[nameLength] = '\0';
        _name = QString((QChar *)tempName, nameLength);
    }

    //qDebug() << "New file named" << _name << "with flags" << _flags << "and crated at" << _created;
}

bool XtafFile::isDeleted()
{
    return deleted;
}

XtafFile::XtafFile(XtafFile *parent, QString name, quint16 flags, quint32 startCluster, quint32 fileSize)
    : _name(name)
{
    this->_parent = parent;
    this->_startCluster = startCluster;
    this->fileSize = fileSize;
    this->_flags = flags;
}


const QString &XtafFile::name()
{
    return _name;
}

const QDateTime &XtafFile::created()
{
    return _created;
}

const QDateTime &XtafFile::accessed()
{
    return _accessed;
}

const QDateTime &XtafFile::updated()
{
    return _updated;
}

quint32 XtafFile::startCluster()
{
    return _startCluster;
}

quint32 XtafFile::parentCluster()
{
    if (_parent)
        return _parent->startCluster();
    return 0;
}

XtafFile *XtafFile::parent()
{
    return _parent;
}

quint32 XtafFile::size()
{
    return fileSize;
}


XtafDirectory::XtafDirectory(XtafFile *parent, XtafFsys *filesystem, struct xtaf_dir_entry *startRec)
    :
      XtafFile(parent, startRec), fs(filesystem), dirIsLoaded(false)
{
}

XtafDirectory::XtafDirectory(XtafFile *parent, XtafFsys *filesystem, quint32 startCluster, QString name)
    :
      XtafFile(parent, name, 0, startCluster, 0),
      fs(filesystem), dirIsLoaded(false)
{
}

int XtafDirectory::loadDirectory()
{
    if (dirIsLoaded)
        return 1;

    QChar cluster[fs->clusterSize()];
    quint32 currentCluster = startCluster();
    do {
        struct xtaf_dir_entry *rec;
        fs->readCluster(cluster, currentCluster);
        rec = (struct xtaf_dir_entry *)cluster;

        while ((QChar *)rec-cluster < fs->clusterSize()) {

            if (rec->access_date == 0xffff
            && rec->access_time == 0xffff
            && rec->update_date == 0xffff
            && rec->update_time == 0xffff
            && rec->create_date == 0xffff
            && rec->create_time == 0xffff
            && rec->file_size == 0xffffffff
            && rec->start_cluster ==  0xffffffff
            && rec->file_flags == 0xff
            && rec->name_len == 0xff
            )
                goto finished;

			if (qFromBigEndian(rec->start_cluster) > fs->partitionSize()) {
                qDebug() << "Exceeded filesystem size";
                goto finished;
            }

            if (rec->name_len > sizeof(rec->filename) && rec->name_len != 0xe5) {
                qDebug() << "Invalid name length:" << rec->name_len << "on file" << QString((const char *)rec->filename);
                goto finished;
            }

            // This filename entry *looks* sane, at least...
            if (rec->file_flags & FF_DIR)
                _entries.append(new XtafDirectory(this, fs, rec));
            else
                _entries.append(new XtafFile(this, rec));
            rec++;
        }
        qDebug() << "Reading next directory cluster...";
        currentCluster = fs->nextCluster(currentCluster);
    } while (currentCluster && currentCluster != 0xffffffff);

finished:

    dirIsLoaded = true;
    return 1;
}

XtafDirectory::~XtafDirectory()
{
    while (!_entries.isEmpty())
        delete _entries.takeFirst();
}

const QList<XtafFile *> &XtafDirectory::entries()
{
    if (!dirIsLoaded)
        loadDirectory();
    return _entries;
}






XtafFsys::XtafFsys()
{
    xtaf = NULL;
    root = NULL;
    cwd = NULL;
    part = NULL;
}

int XtafFsys::clusterSize()
{
    return xtaf->cluster_size;
}

int XtafFsys::readCluster(void *data, quint32 cluster)
{
    quint64 start_offset = 0x1000+xtaf->chainmap_size;

    start_offset += ((cluster-1)*xtaf->cluster_size);
    return part->read(start_offset, data, xtaf->cluster_size);
}

quint64 XtafFsys::partitionSize()
{
    return part->length();
}

quint32 XtafFsys::nextCluster(quint32 cluster) {
    if (xtaf->entry_size == 2) {
        uint16_t *chainmap = (uint16_t *)xtaf->chainmap;
        return (uint32_t)chainmap[cluster];
    }
    else if (xtaf->entry_size == 4) {
        uint32_t *chainmap = (uint32_t *)xtaf->chainmap;
        return (uint32_t)chainmap[cluster];
    }
    qDebug() << "Severe error: Unknown entry size" << xtaf->entry_size;
    return 0xffffffff;
}

int XtafFsys::setPartition(XtafPart *new_partition)
{
    part = new_partition;
    return reloadPartition();
}

int XtafFsys::setPartition(int partNum)
{
    part->setPartition(partNum);
    return reloadPartition();
}

int XtafFsys::reloadPartition()
{
    if (xtaf != NULL) {
        if (xtaf->chainmap)
            free(xtaf->chainmap);
        free(xtaf);
        if (root != NULL)
            delete root;
        root = NULL;
        xtaf = NULL;
    }

    if (part->format() != XtafPart::XTAF) {
        qDebug() << "Partition isn't formatted XTAF";
        part = NULL;
        return -1;
    }

    xtaf = (struct xtaf *)malloc(sizeof(struct xtaf));
    if (!xtaf) {
        qDebug() << "Couldn't allocate XTAF";
        return -1;
    }
    memset(xtaf, 0, sizeof(*xtaf));

    part->read(0, xtaf->magic, 4);
    if (memcmp(xtaf->magic, "XTAF", sizeof(xtaf->magic))) {
        qDebug() << "Magic doesn't match XTAF!" << QString((QChar *)(xtaf->magic), 4);
        free(xtaf);
        return -1;
    }

    part->read(4, &xtaf->id, 4);
    part->read(8, &xtaf->spc, 4);
    part->read(12, &xtaf->rdc, 4);
    xtaf->id = qFromBigEndian(xtaf->id);
    xtaf->spc = qFromBigEndian(xtaf->spc);
    xtaf->rdc = qFromBigEndian(xtaf->rdc);
    xtaf->current_dir = NULL;
    xtaf->length = part->length();

    xtaf->cluster_size = xtaf->spc * 512;
    xtaf->cluster_count = xtaf->length / xtaf->cluster_size;
    xtaf->entry_size = (xtaf->cluster_count>=0xfff0?4:2);
    xtaf->chainmap_size = (xtaf->cluster_count*xtaf->entry_size);
    xtaf->chainmap_size = (xtaf->chainmap_size / 4096 + 1) * 4096;

    /*
    qDebug() << "id:" << xtaf->id;
    qDebug() << "spc:" << xtaf->spc;
    qDebug() << "rdc:" << xtaf->rdc;
    qDebug() << "Partition size:" << xtaf->length;
    qDebug() << "Sectors per cluster:" << xtaf->spc;
    qDebug() << "Root directory cluster:" << xtaf->rdc;
    qDebug() << "Cluster size:" << xtaf->cluster_size;
    qDebug() << "Number of clusters:" << xtaf->cluster_count;
    qDebug() << "Chainmap entries are" << xtaf->entry_size << "bytes";
    qDebug() << "Chainmap size:" << xtaf->chainmap_size << "bytes";
    */

    xtaf->chainmap = (uint8_t *)malloc(xtaf->chainmap_size);
    part->read(0x1000, xtaf->chainmap, xtaf->chainmap_size);

    root = new XtafDirectory(NULL, this, xtaf->rdc, QString("Root"));

    return 0;
}

XtafDirectory &XtafFsys::rootDirectory()
{
    return *root;
}
