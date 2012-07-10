#ifndef XTAFFSYS_H
#define XTAFFSYS_H

#include <QDateTime>
#include "xtafpart.h"

struct xtaf;
struct xtaf_dir;
struct xtaf_dir_entry;

class XtafDirectory;
class XtafFile;


#define FF_DIR 16

class XtafFsys : public QObject
{
    Q_OBJECT

public:
    XtafFsys();
    int setPartition(XtafPart *partition);
    int setPartition(int partNum);
    XtafDirectory &rootDirectory();
    int clusterSize();
    int readCluster(void *data, quint32 cluster);
    quint32 nextCluster(quint32 cluster);

private:
    XtafPart *part;
    struct xtaf *xtaf;
    XtafDirectory *cwd;
    XtafDirectory *root;

    int reloadPartition();
};




class XtafFile : public QObject {
    Q_OBJECT;

public:
    XtafFile(XtafFile *parent, const struct xtaf_dir_entry *entry);
    XtafFile(XtafFile *parent, QString name, quint16 flags, quint32 startCluster, quint32 fileSize);
    virtual bool isDirectory() { return false; }
    bool isDeleted();

    quint32 startCluster();
    quint32 parentCluster();
    XtafFile *parent();
    quint32 size();
    const QString &name();
    const QDateTime &created();
    const QDateTime &accessed();
    const QDateTime &updated();

private:
    XtafFile *_parent;
    QString _name;
    quint32 fileSize;
    quint16 _flags;
    bool deleted;

    QDateTime _created;
    QDateTime _accessed;
    QDateTime _updated;

    quint32 _startCluster;
    quint32 _parentCluster;
};


class XtafDirectory : public XtafFile {
    Q_OBJECT

public:
    XtafDirectory(XtafFile *parent, XtafFsys *filesystem, quint32 startCluster, QString name = QString("Root"));
    XtafDirectory(XtafFile *parent, XtafFsys *filesystem, struct xtaf_dir_entry *rec);
    ~XtafDirectory();
    const QList<XtafFile *> &entries();
    virtual bool isDirectory() { return true; };

private:
    XtafFsys *fs;
    QList<XtafFile *> _entries;
    quint32 parentCluster;
    bool dirIsLoaded;

    int loadDirectory();
};



#endif // XTAFFSYS_H
