#pragma once

#include "analysis/daysummary.h"
#include "ble/ringlink.h"
#include "ble/ringscanner.h"
#include "secrets/keyvault.h"
#include "store/ringstore.h"

#include <QFutureWatcher>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

#include <memory>

// Everything QML talks to. One ring, one conversation at a time.
//
// The conversation with a ring (pair, connect, authenticate, sync) is written as
// a straight-line function that blocks inside nested event loops, see RingLink.
// That is why every entry point below that touches the ring begins with
// beginOperation(): a second tap while the first is running would re-enter the
// same function.
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(bool hasRing READ hasRing NOTIFY ringChanged)
    Q_PROPERTY(QString ringName READ ringName NOTIFY ringChanged)
    Q_PROPERTY(QString ringSerial READ ringSerial NOTIFY ringChanged)
    Q_PROPERTY(QString ringFirmware READ ringFirmware NOTIFY ringChanged)
    Q_PROPERTY(double lastSyncMs READ lastSyncMs NOTIFY ringChanged)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY ringChanged)
    Q_PROPERTY(double batteryMs READ batteryMs NOTIFY ringChanged)
    Q_PROPERTY(int summaryVersion READ summaryVersion NOTIFY summaryChanged)
    Q_PROPERTY(bool summaryBusy READ summaryBusy NOTIFY summaryBusyChanged)
    Q_PROPERTY(int profileAge READ profileAge NOTIFY profileChanged)
    Q_PROPERTY(double profileWeight READ profileWeight NOTIFY profileChanged)
    Q_PROPERTY(int syncedEvents READ syncedEvents NOTIFY statusChanged)
    Q_PROPERTY(bool keyStored READ keyStored NOTIFY ringChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    bool busy() const { return m_busy; }
    QString status() const { return m_status; }
    QString lastError() const { return m_lastError; }
    bool scanning() const { return m_scanner.discovering(); }
    QVariantList devices() const;
    bool hasRing() const { return !m_ring.address.isEmpty(); }
    QString ringName() const { return m_ring.name; }
    QString ringSerial() const { return m_ring.serial; }
    QString ringFirmware() const { return m_ring.firmware; }
    double lastSyncMs() const { return static_cast<double>(m_ring.lastSyncUnix) * 1000.0; }
    int batteryPercent() const { return m_batteryPercent; }
    double batteryMs() const { return static_cast<double>(m_batteryUnix) * 1000.0; }
    int summaryVersion() const { return m_summaryVersion; }
    bool summaryBusy() const { return m_summaryBusy; }
    int profileAge() const { return m_age; }
    double profileWeight() const { return m_weightKg; }
    int syncedEvents() const { return m_syncedEvents; }
    bool keyStored() const { return m_keyStored; }

    // ---- ring discovery and set-up ----
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();

    // Factory-reset ring: pair, install a fresh key, keep it in the vault, sync.
    Q_INVOKABLE void setUpNewRing(const QString &devicePath);
    // Ring that already has a key: store the one the user supplies, then sync.
    // Returns false (with lastError set) if the text is not a 16-byte hex key.
    Q_INVOKABLE bool useExistingKey(const QString &devicePath, const QString &hexKey);
    Q_INVOKABLE void syncNow();
    Q_INVOKABLE void forgetRing();
    // Hex of the stored key for the user's own backup; empty if there is none.
    Q_INVOKABLE QString keyHex();

    // ---- what to show ----
    Q_INVOKABLE QVariantMap overview() const;
    Q_INVOKABLE QVariantList nights(int count) const;
    Q_INVOKABLE QVariantMap night(const QString &date) const;
    Q_INVOKABLE QVariantList days(int count) const;
    Q_INVOKABLE QVariantMap day(const QString &date) const;
    // One value per night, oldest first: {date, value} for a metric key.
    Q_INVOKABLE QVariantList trend(const QString &metric, int count) const;
    // Counts per event type, for the diagnostics page.
    Q_INVOKABLE QVariantList eventStats() const;
    Q_INVOKABLE QString databasePath() const { return m_store.path(); }

    Q_INVOKABLE void setProfile(int age, double weightKg);
    Q_INVOKABLE void clearError();

signals:
    void busyChanged();
    void statusChanged();
    void lastErrorChanged();
    void scanningChanged();
    void devicesChanged();
    void ringChanged();
    void summaryChanged();
    void summaryBusyChanged();
    void profileChanged();
    // Emitted when set-up or a sync is over, with the outcome.
    void operationFinished(bool ok);
    // The ring now has a key we generated: the UI should offer a backup.
    void keyInstalled();

private slots:
    void onDeviceUpdated(const RingScanner::Device &device);
    void onScannerError(const QString &message);

private:
    bool beginOperation(const QString &what);
    void endOperation(bool ok, const QString &error);
    void setStatus(const QString &text);
    void fail(const QString &message);

    // Blocking vault access: nested event loops around the async callbacks.
    bool loadKey(const QString &address, Oura::AesKey &key, QString *error = nullptr);
    bool saveKey(const QString &address, const Oura::AesKey &key, QString *error = nullptr);
    void deleteKey(const QString &address);

    QString ensureOpen(const QString &devicePath);
    // From an open, authenticated link: time, features, battery and history.
    QString runSync(const QString &address, Oura::RingSession &session);
    QString authenticated(const QString &address, Oura::RingSession &session);
    void rememberRing(const QString &devicePath, const QString &fallbackName);

    void reloadRing();
    void refreshSummary();

    RingScanner m_scanner;
    RingLink m_link;
    RingStore m_store;
    KeyVault m_vault;

    QList<RingScanner::Device> m_devices;
    RingStore::RingRecord m_ring;
    bool m_keyStored = false;
    int m_batteryPercent = -1;
    qint64 m_batteryUnix = 0;

    bool m_busy = false;
    QString m_status;
    QString m_lastError;
    int m_syncedEvents = 0;

    int m_age = 40;
    double m_weightKg = 75;

    std::shared_ptr<const Oura::Summary> m_summary;
    int m_summaryVersion = 0;
    bool m_summaryBusy = false;
    bool m_summaryStale = false;
    QFutureWatcher<std::shared_ptr<const Oura::Summary>> m_summaryWatcher;
};
