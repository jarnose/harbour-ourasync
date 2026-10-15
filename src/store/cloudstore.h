#pragma once

#include "cloud/clouddoc.h"

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <vector>

// Oura's cloud documents, as the app keeps them: account-level, not tied to a
// ring, in the same database file as the ring history.
//
// Every function opens its own connection and closes it again. A QSqlDatabase
// belongs to the thread that made it, and imports run on a worker thread while
// the interface thread keeps its own connection open; SQLite in WAL mode lets
// the two coexist.
namespace CloudStore {

void ensureSchema(const QString &databasePath);

// Upserts documents (kind + id is the identity: a re-import replaces) and heart
// rate samples in one transaction.
bool store(const QString &databasePath, const std::vector<Oura::CloudDoc> &docs,
           const std::vector<Oura::CloudHrPoint> &hr, QString *error);

struct Loaded
{
    std::vector<Oura::CloudDoc> docs;
    std::vector<Oura::CloudHrPoint> hr;
};

// Documents dated `sinceDay` (YYYY-MM-DD) or later; heart rate since the unix
// time `hrSinceUtc`.
Loaded load(const QString &databasePath, const QString &sinceDay, qint64 hrSinceUtc);

// {docs: {kind: count}, from: day, to: day, hrSamples: n}
QVariantMap stats(const QString &databasePath);

bool clear(const QString &databasePath);

} // namespace CloudStore
