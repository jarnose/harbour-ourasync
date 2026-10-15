#include "appcontroller.h"

#include "ring/events.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QTimeZone>
#include <QTimer>
#include <QtConcurrent>

#include <algorithm>
#include <cmath>

using namespace Oura;

namespace {

QString secretName(const QString &address)
{
    return QStringLiteral("ringkey-") + address.toLower();
}

QString toHex(const AesKey &key)
{
    return QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(key.data()), 16).toHex());
}

bool parseKey(const QString &text, AesKey &key)
{
    // Accept what people paste: spaces, colons and dashes between bytes.
    QString clean;
    for (const QChar c : text)
        if (c.isLetterOrNumber())
            clean.append(c);
    if (clean.size() != 32)
        return false;
    const QByteArray raw = QByteArray::fromHex(clean.toLatin1());
    if (raw.size() != 16 || raw.toHex() != clean.toLower().toLatin1())
        return false;
    std::copy(raw.begin(), raw.end(), key.begin());
    return true;
}

// A key from the system's entropy pool. Not QRandomGenerator, which arrived in
// Qt 5.10; Sailfish OS ships 5.6.
bool randomKey(AesKey &key)
{
    QFile urandom(QStringLiteral("/dev/urandom"));
    if (!urandom.open(QIODevice::ReadOnly))
        return false;
    const QByteArray bytes = urandom.read(16);
    if (bytes.size() != 16)
        return false;
    std::copy(bytes.begin(), bytes.end(), key.begin());
    return true;
}

qint64 nowUnix()
{
    return QDateTime::currentMSecsSinceEpoch() / 1000;
}

int localOffsetSeconds(int64_t utc)
{
    return QTimeZone::systemTimeZone().offsetFromUtc(QDateTime::fromMSecsSinceEpoch(utc * 1000));
}

int8_t timezoneHalfHours()
{
    const int half = localOffsetSeconds(nowUnix()) / 1800;
    return static_cast<int8_t>(std::max(-48, std::min(48, half)));
}

// A series thinned to at most `maxPoints` by averaging neighbours, as parallel
// arrays: {t: [ms...], v: [...]}. The charts draw a few hundred points; a night
// of heart-rate samples is several thousand.
QVariantMap seriesMap(const std::vector<TimedValue> &s, size_t maxPoints = 240)
{
    QVariantList t, v;
    if (!s.empty()) {
        const size_t group = std::max<size_t>(1, (s.size() + maxPoints - 1) / maxPoints);
        for (size_t i = 0; i < s.size(); i += group) {
            double sumT = 0, sumV = 0;
            size_t n = 0;
            for (size_t k = i; k < std::min(s.size(), i + group); ++k, ++n) {
                sumT += static_cast<double>(s[k].utc);
                sumV += s[k].value;
            }
            t.append(sumT / static_cast<double>(n) * 1000.0);
            v.append(sumV / static_cast<double>(n));
        }
    }
    QVariantMap m;
    m.insert(QStringLiteral("t"), t);
    m.insert(QStringLiteral("v"), v);
    return m;
}

QVariant opt(const std::optional<double> &v)
{
    return v ? QVariant(*v) : QVariant();
}

QVariantMap nightCompact(const Night &n)
{
    QVariantMap m;
    m.insert(QStringLiteral("date"), QString::fromStdString(n.date));
    m.insert(QStringLiteral("startMs"), static_cast<double>(n.startUnix) * 1000.0);
    m.insert(QStringLiteral("endMs"), static_cast<double>(n.endUnix) * 1000.0);
    m.insert(QStringLiteral("main"), n.main);
    m.insert(QStringLiteral("staged"), n.staged);
    m.insert(QStringLiteral("asleepEstimated"), n.asleepEstimated);
    m.insert(QStringLiteral("approximateClock"), n.approximateClock);
    m.insert(QStringLiteral("inBedMin"), n.inBedMin);
    m.insert(QStringLiteral("asleepMin"), n.asleepMin);
    m.insert(QStringLiteral("score"), n.score.valid ? QVariant(n.score.score) : QVariant());
    m.insert(QStringLiteral("efficiency"), opt(n.efficiencyPct));
    m.insert(QStringLiteral("restingHr"), opt(n.restingHr));
    m.insert(QStringLiteral("hrv"), opt(n.avgHrv));
    m.insert(QStringLiteral("breath"), opt(n.avgBreath));
    m.insert(QStringLiteral("spo2"), opt(n.avgSpo2));
    m.insert(QStringLiteral("skinTemp"), opt(n.skinTemp));
    m.insert(QStringLiteral("tempDeviation"), opt(n.tempDeviation));
    m.insert(QStringLiteral("deepMin"), n.deepMin);
    m.insert(QStringLiteral("lightMin"), n.lightMin);
    m.insert(QStringLiteral("remMin"), n.remMin);
    m.insert(QStringLiteral("awakeMin"), n.awakeMin);
    return m;
}

QVariantMap nightFull(const Night &n)
{
    QVariantMap m = nightCompact(n);
    m.insert(QStringLiteral("latencyMin"), opt(n.latencyMin));
    m.insert(QStringLiteral("wasoMin"), opt(n.wasoMin));
    m.insert(QStringLiteral("awakenings"), n.awakenings ? QVariant(*n.awakenings) : QVariant());

    QVariantList stages;
    for (uint8_t c : n.hypnogram)
        stages.append(static_cast<int>(c));
    m.insert(QStringLiteral("hypnogram"), stages);
    m.insert(QStringLiteral("hypnogramStartMs"), static_cast<double>(n.hypnogramStart) * 1000.0);
    m.insert(QStringLiteral("epochSeconds"), n.epochSeconds);

    m.insert(QStringLiteral("hrSeries"), seriesMap(n.hr));
    m.insert(QStringLiteral("hrvSeries"), seriesMap(n.hrv));
    m.insert(QStringLiteral("breathSeries"), seriesMap(n.breath));
    m.insert(QStringLiteral("tempSeries"), seriesMap(n.temp));
    m.insert(QStringLiteral("spo2Series"), seriesMap(n.spo2));

    QVariantList components;
    for (const ScoreComponent &c : n.score.components) {
        QVariantMap cm;
        cm.insert(QStringLiteral("key"), QString::fromStdString(c.key));
        cm.insert(QStringLiteral("score"), c.score);
        cm.insert(QStringLiteral("weight"), c.weight);
        components.append(cm);
    }
    m.insert(QStringLiteral("components"), components);
    return m;
}

QVariantMap dayCompact(const Day &d)
{
    QVariantMap m;
    m.insert(QStringLiteral("date"), QString::fromStdString(d.date));
    m.insert(QStringLiteral("activeKcal"), d.activeKcal);
    m.insert(QStringLiteral("lightMin"), d.lightMin);
    m.insert(QStringLiteral("moderateMin"), d.moderateMin);
    m.insert(QStringLiteral("vigorousMin"), d.vigorousMin);
    m.insert(QStringLiteral("sedentaryMin"), d.sedentaryMin);
    m.insert(QStringLiteral("coveredMin"), d.coveredMin);
    m.insert(QStringLiteral("avgMet"), opt(d.avgMet));
    double lo = 0, hi = 0;
    bool any = false;
    for (const HourStat &h : d.hr) {
        if (h.count == 0)
            continue;
        lo = any ? std::min(lo, h.min) : h.min;
        hi = any ? std::max(hi, h.max) : h.max;
        any = true;
    }
    m.insert(QStringLiteral("hrMin"), any ? QVariant(lo) : QVariant());
    m.insert(QStringLiteral("hrMax"), any ? QVariant(hi) : QVariant());
    return m;
}

QVariantMap dayFull(const Day &d)
{
    QVariantMap m = dayCompact(d);
    QVariantList hours;
    for (const HourStat &h : d.hr) {
        QVariantMap hm;
        hm.insert(QStringLiteral("count"), h.count);
        hm.insert(QStringLiteral("min"), h.min);
        hm.insert(QStringLiteral("max"), h.max);
        hm.insert(QStringLiteral("mean"), h.mean);
        hours.append(hm);
    }
    m.insert(QStringLiteral("hr"), hours);
    QVariantList met;
    for (double v : d.metByHour)
        met.append(v);
    m.insert(QStringLiteral("metByHour"), met);
    return m;
}

} // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    connect(&m_scanner, &RingScanner::deviceUpdated, this, &AppController::onDeviceUpdated);
    connect(&m_scanner, &RingScanner::errorOccurred, this, &AppController::onScannerError);
    connect(&m_summaryWatcher, &QFutureWatcher<std::shared_ptr<const Oura::Summary>>::finished, this, [this]() {
        m_summary = m_summaryWatcher.result();
        m_summaryBusy = false;
        ++m_summaryVersion;
        emit summaryBusyChanged();
        emit summaryChanged();
        if (m_summaryStale) {
            m_summaryStale = false;
            refreshSummary();
        }
    });

    QString error;
    if (!m_store.open(&error)) {
        fail(tr("Could not open the local database: %1").arg(error));
        return;
    }
    m_age = m_store.setting(QStringLiteral("age"), QStringLiteral("40")).toInt();
    m_weightKg = m_store.setting(QStringLiteral("weight"), QStringLiteral("75")).toDouble();

    reloadRing();
    // The vault may ask the device lock, and a nested event loop belongs after
    // the application loop has started.
    QTimer::singleShot(0, this, [this]() {
        if (hasRing()) {
            Oura::AesKey key;
            m_keyStored = loadKey(m_ring.address, key);
            emit ringChanged();
        }
        refreshSummary();
    });
}

// ---- state ------------------------------------------------------------------

void AppController::setStatus(const QString &text)
{
    if (m_status == text)
        return;
    m_status = text;
    emit statusChanged();
}

void AppController::fail(const QString &message)
{
    m_lastError = message;
    emit lastErrorChanged();
}

void AppController::clearError()
{
    if (m_lastError.isEmpty())
        return;
    m_lastError.clear();
    emit lastErrorChanged();
}

bool AppController::beginOperation(const QString &what)
{
    if (m_busy) {
        fail(tr("Another operation is still running."));
        return false;
    }
    stopScan();
    m_busy = true;
    m_syncedEvents = 0;
    clearError();
    setStatus(what);
    emit busyChanged();
    return true;
}

void AppController::endOperation(bool ok, const QString &error)
{
    m_busy = false;
    if (!ok)
        fail(error);
    setStatus(ok ? tr("Done") : QString());
    emit busyChanged();
    emit operationFinished(ok);
}

void AppController::reloadRing()
{
    const QList<RingStore::RingRecord> all = m_store.rings();
    m_ring = all.isEmpty() ? RingStore::RingRecord() : all.first();
    if (hasRing()) {
        const QVariantMap battery = m_store.latestReading(m_ring.address, QStringLiteral("battery"));
        m_batteryPercent = battery.isEmpty() ? -1 : static_cast<int>(battery.value(QStringLiteral("value")).toDouble());
        m_batteryUnix = battery.value(QStringLiteral("unix")).toLongLong();
    } else {
        m_batteryPercent = -1;
        m_batteryUnix = 0;
        m_keyStored = false;
    }
    emit ringChanged();
}

// ---- discovery --------------------------------------------------------------

QVariantList AppController::devices() const
{
    QVariantList out;
    for (const RingScanner::Device &d : m_devices) {
        QVariantMap m;
        m.insert(QStringLiteral("path"), d.objectPath);
        m.insert(QStringLiteral("address"), d.address);
        m.insert(QStringLiteral("name"), d.name);
        m.insert(QStringLiteral("paired"), d.paired);
        m.insert(QStringLiteral("connected"), d.connected);
        m.insert(QStringLiteral("rssi"), d.rssi);
        m.insert(QStringLiteral("known"), d.address.compare(m_ring.address, Qt::CaseInsensitive) == 0);
        out.append(m);
    }
    return out;
}

void AppController::startScan()
{
    if (m_busy)
        return;
    m_scanner.refresh();
    m_scanner.startDiscovery();
    emit scanningChanged();
}

void AppController::stopScan()
{
    if (!m_scanner.discovering())
        return;
    m_scanner.stopDiscovery();
    emit scanningChanged();
}

void AppController::onDeviceUpdated(const RingScanner::Device &device)
{
    for (RingScanner::Device &d : m_devices) {
        if (d.objectPath == device.objectPath) {
            d = device;
            emit devicesChanged();
            return;
        }
    }
    m_devices.append(device);
    emit devicesChanged();
}

void AppController::onScannerError(const QString &message)
{
    fail(message);
}

// ---- the vault, awaited -----------------------------------------------------

namespace {

constexpr int kVaultTimeoutMs = 60000; // the device lock may ask for a code

// What a blocking vault call waits on. Shared by pointer with the callbacks,
// and `loop` is cleared before the caller returns: if the wait times out, a
// late reply must find nothing to wake rather than a destroyed event loop.
struct VaultWait
{
    bool done = false;
    bool ok = false;
    QByteArray data;
    QString error;
    QEventLoop *loop = nullptr;

    void finish()
    {
        done = true;
        if (loop)
            loop->quit();
    }

    // Runs `loop` until finish() or the timeout.
    void wait()
    {
        if (done)
            return;
        QEventLoop eventLoop;
        QTimer timeout;
        timeout.setSingleShot(true);
        QObject::connect(&timeout, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
        loop = &eventLoop;
        timeout.start(kVaultTimeoutMs);
        eventLoop.exec();
        loop = nullptr;
    }
};

} // namespace

bool AppController::loadKey(const QString &address, Oura::AesKey &key, QString *error)
{
    auto state = std::make_shared<VaultWait>();
    // The collection must exist before anything is read from it; creating it
    // again is reported as success.
    m_vault.ensureCollection([this, state, address](bool ok, const QString &err) {
        if (!ok) {
            state->error = err;
            state->finish();
            return;
        }
        m_vault.loadSecret(secretName(address), [state](bool ok2, const QByteArray &data, const QString &err2) {
            state->ok = ok2;
            state->data = data;
            state->error = err2;
            state->finish();
        });
    });
    state->wait();

    if (!state->done || !state->ok || state->data.size() != 16) {
        if (error)
            *error = state->done ? state->error : tr("The secrets vault did not answer.");
        return false;
    }
    std::copy(state->data.begin(), state->data.end(), key.begin());
    return true;
}

bool AppController::saveKey(const QString &address, const Oura::AesKey &key, QString *error)
{
    auto state = std::make_shared<VaultWait>();
    const QByteArray data(reinterpret_cast<const char *>(key.data()), 16);
    m_vault.ensureCollection([this, state, address, data](bool ok, const QString &err) {
        if (!ok) {
            state->error = err;
            state->finish();
            return;
        }
        m_vault.storeSecret(secretName(address), data, [state](bool ok2, const QString &err2) {
            state->ok = ok2;
            state->error = err2;
            state->finish();
        });
    });
    state->wait();

    if (!state->done || !state->ok) {
        if (error)
            *error = state->done ? state->error : tr("The secrets vault did not answer.");
        return false;
    }
    return true;
}

void AppController::deleteKey(const QString &address)
{
    auto state = std::make_shared<VaultWait>();
    m_vault.deleteSecret(secretName(address), [state](bool, const QString &) { state->finish(); });
    state->wait();
}

QString AppController::keyHex()
{
    if (!hasRing())
        return QString();
    Oura::AesKey key;
    if (!loadKey(m_ring.address, key))
        return QString();
    return toHex(key);
}

// ---- the ring conversation --------------------------------------------------

QString AppController::ensureOpen(const QString &devicePath)
{
    setStatus(tr("Connecting to the ring…"));
    return m_link.open(devicePath);
}

QString AppController::authenticated(const QString &address, Oura::RingSession &session)
{
    Oura::AesKey key;
    QString vaultError;
    if (!loadKey(address, key, &vaultError))
        return tr("The ring's key is not in the secrets vault (%1). Set the ring up again, or enter its key.")
                .arg(vaultError.isEmpty() ? tr("not found") : vaultError);

    setStatus(tr("Authenticating…"));
    Oura::AuthResult result = Oura::AuthResult::Unknown;
    const Oura::Outcome o = session.authenticate(key, &result);
    if (o.ok)
        return QString();
    if (result == Oura::AuthResult::InFactoryReset)
        return tr("The ring has been factory reset and has no key. Set it up again.");
    if (result == Oura::AuthResult::AuthenticationError)
        return tr("The ring rejected the stored key. Another app has probably set the ring up since; "
                  "a factory reset and a new set-up are needed.");
    return tr("Authentication failed: %1").arg(QString::fromStdString(o.error));
}

QString AppController::runSync(const QString &address, Oura::RingSession &session)
{
    // Ring details first: they cost nothing and make the record readable.
    {
        RingStore::RingRecord rec;
        m_store.ring(address, &rec);
        DeviceInfo info;
        if (session.firmware(info).ok)
            rec.firmware = QString::fromStdString(info.firmwareVersion);
        std::string text;
        if (session.serial(text).ok)
            rec.serial = QString::fromStdString(text);
        if (session.hardwareId(text).ok)
            rec.hardware = QString::fromStdString(text);
        m_store.saveRing(rec);
    }

    setStatus(tr("Preparing the ring…"));
    Outcome o = session.setupAppStream();
    if (!o.ok)
        return tr("Could not prepare the ring: %1").arg(QString::fromStdString(o.error));

    // Time first, so that the ring writes a time_sync event into its history
    // before we fetch it: that event is what dates everything else.
    o = session.syncTime(static_cast<uint64_t>(nowUnix()), timezoneHalfHours());
    if (!o.ok)
        return tr("Could not set the ring's clock: %1").arg(QString::fromStdString(o.error));
    // Let the ring finish any sleep analysis it owes, so a night that ended
    // this morning is in this download.
    session.checkSleepAnalysis();

    Battery battery;
    if (session.battery(battery).ok) {
        m_store.addReading(address, QStringLiteral("battery"), battery.percent, nowUnix());
        m_batteryPercent = battery.percent;
        m_batteryUnix = nowUnix();
    }

    setStatus(tr("Downloading history…"));
    const qint64 captured = nowUnix();
    DrainResult drained = session.drain(m_store.cursor(address), [&](const std::vector<RingEvent> &events,
                                                                       const SyncProgress &progress) {
        // Events and cursor are committed together or not at all.
        if (!m_store.addEvents(address, events, captured, progress.cursor))
            return false;
        m_syncedEvents = static_cast<int>(progress.eventsSynced);
        setStatus(tr("Downloading history: %1 events").arg(progress.eventsSynced));
        return true;
    });
    if (!drained.status.ok) {
        // Whatever was committed stays; the next sync resumes after it.
        refreshSummary();
        return tr("History download stopped: %1").arg(QString::fromStdString(drained.status.error));
    }

    RingStore::RingRecord rec;
    if (m_store.ring(address, &rec)) {
        rec.lastSyncUnix = nowUnix();
        m_store.saveRing(rec);
    }
    reloadRing();
    refreshSummary();
    return QString();
}

void AppController::rememberRing(const QString &devicePath, const QString &fallbackName)
{
    QString address;
    QString name = fallbackName;
    for (const RingScanner::Device &d : m_devices) {
        if (d.objectPath == devicePath) {
            address = d.address;
            if (!d.name.isEmpty())
                name = d.name;
        }
    }
    if (address.isEmpty()) {
        // Object paths end in dev_AA_BB_CC_DD_EE_FF.
        address = devicePath.section(QLatin1String("/dev_"), -1).replace(QLatin1Char('_'), QLatin1Char(':'));
    }
    RingStore::RingRecord rec;
    if (!m_store.ring(address, &rec)) {
        rec.address = address;
        rec.addedUnix = nowUnix();
    }
    rec.name = name;
    m_store.saveRing(rec);
    reloadRing();
}

void AppController::syncNow()
{
    if (!hasRing()) {
        fail(tr("No ring is set up yet."));
        return;
    }
    if (!beginOperation(tr("Syncing…")))
        return;

    const QString address = m_ring.address;
    QString error;
    const QString path = m_scanner.devicePathForAddress(address);
    if (path.isEmpty()) {
        endOperation(false, tr("Bluetooth does not know this ring any more. Set it up again."));
        return;
    }
    error = ensureOpen(path);
    if (error.isEmpty()) {
        Oura::RingSession session(m_link);
        error = authenticated(address, session);
        if (error.isEmpty())
            error = runSync(address, session);
    }
    // Let go of the ring: it takes one connection at a time, and a connected
    // ring is one the Oura app, or another phone, cannot reach.
    m_link.close(true);
    endOperation(error.isEmpty(), error);
}

void AppController::setUpNewRing(const QString &devicePath)
{
    if (!beginOperation(tr("Pairing…")))
        return;

    QString error = m_link.pair(devicePath);
    if (!error.isEmpty()) {
        endOperation(false, error);
        return;
    }
    rememberRing(devicePath, tr("Oura ring"));
    const QString address = m_ring.address;

    error = ensureOpen(devicePath);
    if (!error.isEmpty()) {
        m_link.close(true);
        endOperation(false, error);
        return;
    }
    Oura::RingSession session(m_link);

    // A ring we already hold the key for needs no new key: prove it first. This
    // is also what makes tapping "set up" twice harmless.
    bool alreadyOurs = false;
    Oura::AesKey oldKey{};
    bool hadOldKey = loadKey(address, oldKey);
    if (hadOldKey) {
        setStatus(tr("Authenticating…"));
        alreadyOurs = session.authenticate(oldKey).ok;
    }

    if (!alreadyOurs) {
        setStatus(tr("Installing the ring's key…"));
        Oura::AesKey key{};
        if (!randomKey(key)) {
            m_link.close(true);
            endOperation(false, tr("Could not read the system's random source."));
            return;
        }
        // Keep the key safe BEFORE the ring is told about it. If the ring took
        // the key and the vault then failed, the ring would be locked to a key
        // that exists nowhere; the other order can only waste a vault entry.
        QString vaultError;
        if (!saveKey(address, key, &vaultError)) {
            m_link.close(true);
            endOperation(false, tr("Could not store the key in the secrets vault: %1").arg(vaultError));
            return;
        }
        const Oura::Outcome set = session.setAuthKey(key);
        if (!set.ok) {
            if (hadOldKey)
                saveKey(address, oldKey);
            else
                deleteKey(address);
            m_link.close(true);
            endOperation(false, tr("The ring would not take a new key: %1\n\nA ring that is still set up "
                                   "with the Oura app must first be factory reset.")
                                        .arg(QString::fromStdString(set.error)));
            return;
        }
        RingStore::RingRecord rec;
        if (m_store.ring(address, &rec)) {
            rec.keyInstalledByUs = true;
            m_store.saveRing(rec);
        }
        m_keyStored = true;
        emit keyInstalled();

        setStatus(tr("Authenticating…"));
        error = authenticated(address, session);
    }
    m_keyStored = true;
    emit ringChanged();

    if (error.isEmpty())
        error = runSync(address, session);
    m_link.close(true);
    endOperation(error.isEmpty(), error);
}

bool AppController::useExistingKey(const QString &devicePath, const QString &hexKey)
{
    Oura::AesKey key{};
    if (!parseKey(hexKey, key)) {
        fail(tr("A key is 32 hexadecimal characters (16 bytes)."));
        return false;
    }
    if (!beginOperation(tr("Storing the key…")))
        return false;

    rememberRing(devicePath, tr("Oura ring"));
    const QString address = m_ring.address;
    QString error;
    if (!saveKey(address, key, &error)) {
        endOperation(false, tr("Could not store the key in the secrets vault: %1").arg(error));
        return false;
    }
    m_keyStored = true;

    setStatus(tr("Pairing…"));
    error = m_link.pair(devicePath);
    if (error.isEmpty())
        error = ensureOpen(devicePath);
    if (error.isEmpty()) {
        Oura::RingSession session(m_link);
        error = authenticated(address, session);
        if (error.isEmpty())
            error = runSync(address, session);
    }
    m_link.close(true);
    endOperation(error.isEmpty(), error);
    return error.isEmpty();
}

void AppController::forgetRing()
{
    if (m_busy || !hasRing())
        return;
    const QString address = m_ring.address;
    const QString path = m_scanner.devicePathForAddress(address);
    if (!path.isEmpty())
        m_scanner.removeDevice(path);
    deleteKey(address);
    m_store.forgetRing(address);
    m_summary.reset();
    m_devices.clear();
    reloadRing();
    ++m_summaryVersion;
    emit summaryChanged();
    emit devicesChanged();
}

// ---- profile ----------------------------------------------------------------

void AppController::setProfile(int age, double weightKg)
{
    m_age = std::max(10, std::min(110, age));
    m_weightKg = std::max(25.0, std::min(250.0, weightKg));
    m_store.setSetting(QStringLiteral("age"), QString::number(m_age));
    m_store.setSetting(QStringLiteral("weight"), QString::number(m_weightKg));
    emit profileChanged();
    refreshSummary();
}

// ---- summaries --------------------------------------------------------------

void AppController::refreshSummary()
{
    if (!hasRing()) {
        m_summary.reset();
        ++m_summaryVersion;
        emit summaryChanged();
        return;
    }
    if (m_summaryBusy) {
        m_summaryStale = true;
        return;
    }
    m_summaryBusy = true;
    emit summaryBusyChanged();

    const QString path = m_store.path();
    const QString address = m_ring.address;
    const double age = m_age;
    const double weight = m_weightKg;
    // About three months. The history is read by when it was downloaded, which
    // is a safe bound on when it happened.
    const qint64 since = nowUnix() - 100LL * 86400;

    m_summaryWatcher.setFuture(QtConcurrent::run([path, address, age, weight, since]() {
        const std::vector<StoredEvent> events = RingStore::readEvents(path, address, since);
        AnalysisOptions options;
        options.ageYears = age;
        options.weightKg = weight;
        options.utcOffset = [](int64_t t) { return localOffsetSeconds(t); };
        return std::shared_ptr<const Summary>(std::make_shared<Summary>(summarize(events, options)));
    }));
}

QVariantMap AppController::overview() const
{
    QVariantMap m;
    m.insert(QStringLiteral("hasData"), m_summary && (!m_summary->nights.empty() || !m_summary->days.empty()));
    if (!m_summary)
        return m;
    const Summary &s = *m_summary;

    for (auto it = s.nights.rbegin(); it != s.nights.rend(); ++it) {
        if (it->main) {
            m.insert(QStringLiteral("night"), nightCompact(*it));
            break;
        }
    }
    if (!s.days.empty()) {
        const std::string today = localDate(nowUnix(), localOffsetSeconds(nowUnix()));
        for (auto it = s.days.rbegin(); it != s.days.rend(); ++it) {
            if (it->date == today) {
                m.insert(QStringLiteral("today"), dayFull(*it));
                break;
            }
        }
        m.insert(QStringLiteral("latestDay"), dayCompact(s.days.back()));
    }
    if (s.latestHr) {
        m.insert(QStringLiteral("latestHr"), *s.latestHr);
        m.insert(QStringLiteral("latestHrMs"), static_cast<double>(s.latestHrUnix) * 1000.0);
    }
    if (s.ringBatteryPercent) {
        m.insert(QStringLiteral("logBattery"), *s.ringBatteryPercent);
        m.insert(QStringLiteral("logBatteryMs"), static_cast<double>(s.ringBatteryUnix) * 1000.0);
    }
    m.insert(QStringLiteral("nightCount"), static_cast<int>(s.nights.size()));
    m.insert(QStringLiteral("unplacedEvents"), s.unplacedEvents);
    return m;
}

QVariantList AppController::nights(int count) const
{
    QVariantList out;
    if (!m_summary)
        return out;
    const auto &all = m_summary->nights;
    for (auto it = all.rbegin(); it != all.rend() && out.size() < count; ++it)
        out.append(nightCompact(*it));
    return out;
}

QVariantMap AppController::night(const QString &date) const
{
    if (!m_summary)
        return QVariantMap();
    const std::string key = date.toStdString();
    const Night *best = nullptr;
    for (const Night &n : m_summary->nights)
        if (n.date == key && (!best || n.asleepMin > best->asleepMin))
            best = &n;
    return best ? nightFull(*best) : QVariantMap();
}

QVariantList AppController::days(int count) const
{
    QVariantList out;
    if (!m_summary)
        return out;
    const auto &all = m_summary->days;
    for (auto it = all.rbegin(); it != all.rend() && out.size() < count; ++it)
        out.append(dayCompact(*it));
    return out;
}

QVariantMap AppController::day(const QString &date) const
{
    if (!m_summary)
        return QVariantMap();
    const std::string key = date.toStdString();
    for (const Day &d : m_summary->days)
        if (d.date == key)
            return dayFull(d);
    return QVariantMap();
}

QVariantList AppController::trend(const QString &metric, int count) const
{
    QVariantList out;
    if (!m_summary)
        return out;

    std::vector<std::pair<std::string, double>> points;
    for (const Night &n : m_summary->nights) {
        if (!n.main)
            continue;
        std::optional<double> v;
        if (metric == QLatin1String("score") && n.score.valid)
            v = n.score.score;
        else if (metric == QLatin1String("asleep"))
            v = n.asleepMin / 60.0;
        else if (metric == QLatin1String("efficiency"))
            v = n.efficiencyPct;
        else if (metric == QLatin1String("restingHr"))
            v = n.restingHr;
        else if (metric == QLatin1String("hrv"))
            v = n.avgHrv;
        else if (metric == QLatin1String("breath"))
            v = n.avgBreath;
        else if (metric == QLatin1String("spo2"))
            v = n.avgSpo2;
        else if (metric == QLatin1String("skinTemp"))
            v = n.skinTemp;
        else if (metric == QLatin1String("tempDeviation"))
            v = n.tempDeviation;
        else if (metric == QLatin1String("deep") && n.staged)
            v = n.deepMin;
        else if (metric == QLatin1String("rem") && n.staged)
            v = n.remMin;
        if (v)
            points.emplace_back(n.date, *v);
    }
    const size_t first = points.size() > static_cast<size_t>(count) ? points.size() - static_cast<size_t>(count) : 0;
    for (size_t i = first; i < points.size(); ++i) {
        QVariantMap p;
        p.insert(QStringLiteral("date"), QString::fromStdString(points[i].first));
        p.insert(QStringLiteral("value"), points[i].second);
        out.append(p);
    }
    return out;
}

QVariantList AppController::eventStats() const
{
    QVariantList out;
    if (!hasRing())
        return out;
    for (const auto &entry : m_store.tagCounts(m_ring.address)) {
        QVariantMap m;
        m.insert(QStringLiteral("tag"), entry.first);
        m.insert(QStringLiteral("name"), QString::fromLatin1(Oura::eventName(static_cast<uint8_t>(entry.first))));
        m.insert(QStringLiteral("count"), entry.second);
        out.append(m);
    }
    return out;
}
