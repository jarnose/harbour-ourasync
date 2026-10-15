#include "cloudstore.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace {

// RAII for a private connection; QUuid::WithoutBraces is Qt 5.11, so the braces
// are stripped by hand.
class Connection
{
public:
    explicit Connection(const QString &path)
    {
        QString id = QUuid::createUuid().toString();
        id.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
        m_name = QStringLiteral("ouras-cloud-") + id;
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_name);
        db.setDatabaseName(path);
        m_ok = db.open();
        m_error = db.lastError().text();
        if (m_ok) {
            QSqlQuery q(db);
            q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
            q.exec(QStringLiteral("PRAGMA synchronous=NORMAL"));
            q.exec(QStringLiteral("PRAGMA busy_timeout=15000"));
        }
    }
    ~Connection()
    {
        {
            QSqlDatabase db = QSqlDatabase::database(m_name, false);
            if (db.isValid() && db.isOpen())
                db.close();
        }
        QSqlDatabase::removeDatabase(m_name);
    }
    bool ok() const { return m_ok; }
    QString error() const { return m_error; }
    QSqlDatabase db() const { return QSqlDatabase::database(m_name); }

private:
    QString m_name;
    bool m_ok = false;
    QString m_error;
};

void createTables(QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS cloud_docs ("
                          " kind TEXT NOT NULL, id TEXT NOT NULL, day TEXT NOT NULL, json TEXT NOT NULL,"
                          " PRIMARY KEY(kind, id))"));
    q.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS cloud_docs_by_day ON cloud_docs(day)"));
    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS cloud_hr (utc INTEGER PRIMARY KEY, bpm INTEGER NOT NULL)"));
}

} // namespace

namespace CloudStore {

void ensureSchema(const QString &databasePath)
{
    Connection c(databasePath);
    if (c.ok()) {
        QSqlDatabase db = c.db();
        createTables(db);
    }
}

bool store(const QString &databasePath, const std::vector<Oura::CloudDoc> &docs,
           const std::vector<Oura::CloudHrPoint> &hr, QString *error)
{
    Connection c(databasePath);
    if (!c.ok()) {
        if (error)
            *error = c.error();
        return false;
    }
    QSqlDatabase db = c.db();
    createTables(db);
    if (!db.transaction()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    QSqlQuery doc(db);
    doc.prepare(QStringLiteral("INSERT OR REPLACE INTO cloud_docs(kind, id, day, json) VALUES(?,?,?,?)"));
    for (const Oura::CloudDoc &d : docs) {
        doc.addBindValue(QString::fromStdString(d.kind));
        doc.addBindValue(QString::fromStdString(d.id));
        doc.addBindValue(QString::fromStdString(d.day));
        doc.addBindValue(QString::fromStdString(d.json.dump()));
        if (!doc.exec()) {
            if (error)
                *error = doc.lastError().text();
            db.rollback();
            return false;
        }
    }
    QSqlQuery point(db);
    point.prepare(QStringLiteral("INSERT OR REPLACE INTO cloud_hr(utc, bpm) VALUES(?,?)"));
    for (const Oura::CloudHrPoint &p : hr) {
        point.addBindValue(static_cast<qint64>(p.utc));
        point.addBindValue(p.bpm);
        if (!point.exec()) {
            if (error)
                *error = point.lastError().text();
            db.rollback();
            return false;
        }
    }
    if (!db.commit()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }
    return true;
}

Loaded load(const QString &databasePath, const QString &sinceDay, qint64 hrSinceUtc)
{
    Loaded out;
    Connection c(databasePath);
    if (!c.ok())
        return out;
    QSqlDatabase db = c.db();
    createTables(db);

    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare(QStringLiteral("SELECT kind, id, day, json FROM cloud_docs WHERE day >= ? ORDER BY day"));
    q.addBindValue(sinceDay);
    if (q.exec()) {
        while (q.next()) {
            Oura::CloudDoc d;
            d.kind = q.value(0).toString().toStdString();
            d.id = q.value(1).toString().toStdString();
            d.day = q.value(2).toString().toStdString();
            if (auto j = Oura::Json::parse(q.value(3).toString().toStdString())) {
                d.json = std::move(*j);
                out.docs.push_back(std::move(d));
            }
        }
    }
    QSqlQuery h(db);
    h.setForwardOnly(true);
    h.prepare(QStringLiteral("SELECT utc, bpm FROM cloud_hr WHERE utc >= ? ORDER BY utc"));
    h.addBindValue(hrSinceUtc);
    if (h.exec())
        while (h.next())
            out.hr.push_back({ h.value(0).toLongLong(), h.value(1).toInt() });
    return out;
}

QVariantMap stats(const QString &databasePath)
{
    QVariantMap out;
    Connection c(databasePath);
    if (!c.ok())
        return out;
    QSqlDatabase db = c.db();
    createTables(db);

    QVariantMap kinds;
    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT kind, COUNT(*), MIN(day), MAX(day) FROM cloud_docs GROUP BY kind ORDER BY kind"))) {
        QString from, to;
        while (q.next()) {
            kinds.insert(q.value(0).toString(), q.value(1).toInt());
            const QString lo = q.value(2).toString(), hi = q.value(3).toString();
            if (from.isEmpty() || lo < from)
                from = lo;
            if (to.isEmpty() || hi > to)
                to = hi;
        }
        out.insert(QStringLiteral("from"), from);
        out.insert(QStringLiteral("to"), to);
    }
    out.insert(QStringLiteral("docs"), kinds);
    QSqlQuery h(db);
    if (h.exec(QStringLiteral("SELECT COUNT(*) FROM cloud_hr")) && h.next())
        out.insert(QStringLiteral("hrSamples"), h.value(0).toLongLong());
    return out;
}

bool clear(const QString &databasePath)
{
    Connection c(databasePath);
    if (!c.ok())
        return false;
    QSqlDatabase db = c.db();
    createTables(db);
    QSqlQuery q(db);
    return q.exec(QStringLiteral("DELETE FROM cloud_docs")) && q.exec(QStringLiteral("DELETE FROM cloud_hr"));
}

} // namespace CloudStore
