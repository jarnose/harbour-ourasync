#pragma once

#include "analysis/ringclock.h"
#include "ring/protocol.h"

#include <QList>
#include <QObject>
#include <QPair>
#include <QSqlDatabase>
#include <QString>
#include <QVariantMap>

#include <vector>

// Everything the app keeps about rings and their history, in one SQLite file.
//
// Events are stored raw and lossless: the body bytes exactly as the ring sent
// them. Decoding happens when a summary is built, never on the way in, so a
// decoder fixed or added later improves the history already on the phone without
// another sync (and without the ring, which only ever hands over each event
// once).
//
// The file lives under QStandardPaths::AppDataLocation, which Sailjail grants
// for the organisation/application names set in main().
class RingStore : public QObject
{
    Q_OBJECT
public:
    explicit RingStore(QObject *parent = nullptr);
    ~RingStore() override;

    bool open(QString *error = nullptr);

    struct RingRecord
    {
        QString address; // BlueZ MAC, the stable identity
        QString name;
        QString serial;
        QString hardware;
        QString firmware;
        qint64 addedUnix = 0;
        qint64 lastSyncUnix = 0;
        bool keyInstalledByUs = false; // we set the key, rather than importing it
    };

    void saveRing(const RingRecord &ring);
    QList<RingRecord> rings() const;
    bool ring(const QString &address, RingRecord *out) const;
    // Removes the ring and everything synced from it.
    void forgetRing(const QString &address);

    // The ring-clock cursor to continue from, in deciseconds.
    quint32 cursor(const QString &address) const;
    void setCursor(const QString &address, quint32 cursor);

    // Inserts the events and moves the cursor in one transaction: either both
    // happened or neither did, which is what makes an interrupted sync resume
    // instead of losing or repeating a stretch of history. Duplicates (the ring
    // replays its tail) are ignored. Returns false on a database error.
    bool addEvents(const QString &address, const std::vector<Oura::RingEvent> &events, qint64 capturedUnix,
                   quint32 newCursor);

    // Events of a ring in download order, downloaded since `sinceCapturedUnix`.
    // Static, on its own connection: a QSqlDatabase belongs to the thread that
    // made it, and the history is read on a worker thread so that building a
    // summary never stalls the interface.
    static std::vector<Oura::StoredEvent> readEvents(const QString &databasePath, const QString &address,
                                                     qint64 sinceCapturedUnix = 0);
    qint64 eventCount(const QString &address) const;
    // {tag, count} per event type present, for diagnostics.
    QList<QPair<int, qint64>> tagCounts(const QString &address) const;

    void addReading(const QString &address, const QString &kind, double value, qint64 utc);
    QVariantMap latestReading(const QString &address, const QString &kind) const;

    QString setting(const QString &key, const QString &fallback = QString()) const;
    void setSetting(const QString &key, const QString &value);

    // Path of the database file, for diagnostics.
    QString path() const { return m_path; }

private:
    bool exec(const QString &sql, const QVariantList &args = {}) const;

    QString m_connectionName;
    QString m_path;
};
