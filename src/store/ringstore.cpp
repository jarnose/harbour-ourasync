#include "ringstore.h"

#include <QDateTime>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QVariant>

namespace {

// QUuid::WithoutBraces is Qt 5.11; Sailfish OS ships 5.6.
QString uniqueName()
{
    QString id = QUuid::createUuid().toString();
    id.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
    return id;
}

} // namespace

RingStore::RingStore(QObject *parent)
    : QObject(parent)
    , m_connectionName(QStringLiteral("ouras-") + uniqueName())
{
}

RingStore::~RingStore()
{
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if (db.isValid() && db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool RingStore::open(QString *error)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    m_path = dir + QStringLiteral("/ring.db");

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(m_path);
    if (!db.open()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    // WAL: a summary being built on a worker thread must not block a sync's
    // writes, and a crash mid-write must not corrupt the history.
    exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    exec(QStringLiteral("PRAGMA synchronous=NORMAL"));

    const char *schema[] = {
        "CREATE TABLE IF NOT EXISTS rings ("
        " address TEXT PRIMARY KEY, name TEXT, serial TEXT, hardware TEXT, firmware TEXT,"
        " added INTEGER, last_sync INTEGER, key_by_us INTEGER DEFAULT 0)",
        "CREATE TABLE IF NOT EXISTS sync_state ("
        " address TEXT PRIMARY KEY, cursor INTEGER NOT NULL DEFAULT 0)",
        // body is the raw event payload after the 4-byte timestamp.
        "CREATE TABLE IF NOT EXISTS events ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT, address TEXT NOT NULL, tag INTEGER NOT NULL,"
        " ds INTEGER NOT NULL, body BLOB NOT NULL, captured INTEGER NOT NULL,"
        " UNIQUE(address, tag, ds, body))",
        "CREATE INDEX IF NOT EXISTS events_by_ring ON events(address, id)",
        "CREATE TABLE IF NOT EXISTS readings ("
        " address TEXT NOT NULL, kind TEXT NOT NULL, unix INTEGER NOT NULL, value REAL NOT NULL)",
        "CREATE INDEX IF NOT EXISTS readings_by_kind ON readings(address, kind, unix)",
        "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT)",
    };
    for (const char *sql : schema)
        if (!exec(QString::fromLatin1(sql))) {
            if (error)
                *error = db.lastError().text();
            return false;
        }
    return true;
}

bool RingStore::exec(const QString &sql, const QVariantList &args) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(sql);
    for (const QVariant &a : args)
        q.addBindValue(a);
    return q.exec();
}

void RingStore::saveRing(const RingRecord &r)
{
    exec(QStringLiteral("INSERT INTO rings(address, name, serial, hardware, firmware, added, last_sync, key_by_us)"
                        " VALUES(?,?,?,?,?,?,?,?)"
                        " ON CONFLICT(address) DO UPDATE SET name=excluded.name, serial=excluded.serial,"
                        " hardware=excluded.hardware, firmware=excluded.firmware,"
                        " last_sync=excluded.last_sync, key_by_us=excluded.key_by_us"),
         { r.address, r.name, r.serial, r.hardware, r.firmware, r.addedUnix, r.lastSyncUnix,
           r.keyInstalledByUs ? 1 : 0 });
}

static RingStore::RingRecord recordFrom(QSqlQuery &q)
{
    RingStore::RingRecord r;
    r.address = q.value(0).toString();
    r.name = q.value(1).toString();
    r.serial = q.value(2).toString();
    r.hardware = q.value(3).toString();
    r.firmware = q.value(4).toString();
    r.addedUnix = q.value(5).toLongLong();
    r.lastSyncUnix = q.value(6).toLongLong();
    r.keyInstalledByUs = q.value(7).toInt() != 0;
    return r;
}

QList<RingStore::RingRecord> RingStore::rings() const
{
    QList<RingRecord> out;
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    if (q.exec(QStringLiteral("SELECT address,name,serial,hardware,firmware,added,last_sync,key_by_us"
                              " FROM rings ORDER BY added")))
        while (q.next())
            out.append(recordFrom(q));
    return out;
}

bool RingStore::ring(const QString &address, RingRecord *out) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT address,name,serial,hardware,firmware,added,last_sync,key_by_us"
                             " FROM rings WHERE address=?"));
    q.addBindValue(address);
    if (!q.exec() || !q.next())
        return false;
    if (out)
        *out = recordFrom(q);
    return true;
}

void RingStore::forgetRing(const QString &address)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();
    for (const char *table : { "rings", "sync_state", "events", "readings" })
        exec(QStringLiteral("DELETE FROM %1 WHERE address=?").arg(QLatin1String(table)), { address });
    db.commit();
}

void RingStore::checkpoint()
{
    exec(QStringLiteral("PRAGMA wal_checkpoint(TRUNCATE)"));
}

quint32 RingStore::cursor(const QString &address) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT cursor FROM sync_state WHERE address=?"));
    q.addBindValue(address);
    return (q.exec() && q.next()) ? q.value(0).toUInt() : 0;
}

void RingStore::setCursor(const QString &address, quint32 cursor)
{
    exec(QStringLiteral("INSERT INTO sync_state(address, cursor) VALUES(?,?)"
                        " ON CONFLICT(address) DO UPDATE SET cursor=excluded.cursor"),
         { address, static_cast<qint64>(cursor) });
}

bool RingStore::addEvents(const QString &address, const std::vector<Oura::RingEvent> &events,
                          qint64 capturedUnix, quint32 newCursor)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.transaction())
        return false;

    QSqlQuery insert(db);
    insert.prepare(QStringLiteral("INSERT OR IGNORE INTO events(address, tag, ds, body, captured)"
                                  " VALUES(?,?,?,?,?)"));
    for (const Oura::RingEvent &e : events) {
        insert.addBindValue(address);
        insert.addBindValue(static_cast<int>(e.tag));
        insert.addBindValue(static_cast<qint64>(e.timestamp));
        insert.addBindValue(QByteArray(reinterpret_cast<const char *>(e.body.data()),
                                       static_cast<int>(e.body.size())));
        insert.addBindValue(capturedUnix);
        if (!insert.exec()) {
            db.rollback();
            return false;
        }
    }
    QSqlQuery cursorQuery(db);
    cursorQuery.prepare(QStringLiteral("INSERT INTO sync_state(address, cursor) VALUES(?,?)"
                                       " ON CONFLICT(address) DO UPDATE SET cursor=excluded.cursor"));
    cursorQuery.addBindValue(address);
    cursorQuery.addBindValue(static_cast<qint64>(newCursor));
    if (!cursorQuery.exec()) {
        db.rollback();
        return false;
    }
    return db.commit();
}

std::vector<Oura::StoredEvent> RingStore::readEvents(const QString &databasePath, const QString &address,
                                                     qint64 sinceCapturedUnix)
{
    std::vector<Oura::StoredEvent> out;
    const QString connection = QStringLiteral("ouras-read-") + uniqueName();
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        db.setDatabaseName(databasePath);
        db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        if (db.open()) {
            QSqlQuery q(db);
            q.setForwardOnly(true);
            q.prepare(QStringLiteral("SELECT id, tag, ds, body, captured FROM events"
                                     " WHERE address=? AND captured>=? ORDER BY id"));
            q.addBindValue(address);
            q.addBindValue(sinceCapturedUnix);
            if (q.exec()) {
                while (q.next()) {
                    Oura::StoredEvent e;
                    e.id = q.value(0).toLongLong();
                    e.tag = static_cast<uint8_t>(q.value(1).toInt());
                    e.ds = q.value(2).toLongLong();
                    const QByteArray body = q.value(3).toByteArray();
                    e.body.assign(body.begin(), body.end());
                    e.capturedUnix = q.value(4).toLongLong();
                    out.push_back(std::move(e));
                }
            }
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(connection);
    return out;
}

qint64 RingStore::eventCount(const QString &address) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM events WHERE address=?"));
    q.addBindValue(address);
    return (q.exec() && q.next()) ? q.value(0).toLongLong() : 0;
}

QList<QPair<int, qint64>> RingStore::tagCounts(const QString &address) const
{
    QList<QPair<int, qint64>> out;
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT tag, COUNT(*) FROM events WHERE address=? GROUP BY tag ORDER BY tag"));
    q.addBindValue(address);
    if (q.exec())
        while (q.next())
            out.append(qMakePair(q.value(0).toInt(), q.value(1).toLongLong()));
    return out;
}

void RingStore::addReading(const QString &address, const QString &kind, double value, qint64 utc)
{
    exec(QStringLiteral("INSERT INTO readings(address, kind, unix, value) VALUES(?,?,?,?)"),
         { address, kind, utc, value });
}

QVariantMap RingStore::latestReading(const QString &address, const QString &kind) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT unix, value FROM readings WHERE address=? AND kind=?"
                             " ORDER BY unix DESC LIMIT 1"));
    q.addBindValue(address);
    q.addBindValue(kind);
    QVariantMap out;
    if (q.exec() && q.next()) {
        out.insert(QStringLiteral("unix"), q.value(0).toLongLong());
        out.insert(QStringLiteral("value"), q.value(1).toDouble());
    }
    return out;
}

QString RingStore::setting(const QString &key, const QString &fallback) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT value FROM settings WHERE key=?"));
    q.addBindValue(key);
    return (q.exec() && q.next()) ? q.value(0).toString() : fallback;
}

void RingStore::setSetting(const QString &key, const QString &value)
{
    exec(QStringLiteral("INSERT INTO settings(key, value) VALUES(?,?)"
                        " ON CONFLICT(key) DO UPDATE SET value=excluded.value"),
         { key, value });
}
