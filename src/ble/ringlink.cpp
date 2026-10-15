#include "ringlink.h"

#include "bluezcall.h"
#include "bluezmanagedobjects.h"
#include "ring/protocol.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include <memory>

namespace {

const QString kBluezService = QStringLiteral("org.bluez");
const QString kPropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const QString kDeviceInterface = QStringLiteral("org.bluez.Device1");
const QString kServiceInterface = QStringLiteral("org.bluez.GattService1");
const QString kCharInterface = QStringLiteral("org.bluez.GattCharacteristic1");
const QString kObjectManagerInterface = QStringLiteral("org.freedesktop.DBus.ObjectManager");

const QString kErrInProgress = QStringLiteral("org.bluez.Error.InProgress");
const QString kErrAlreadyConnected = QStringLiteral("org.bluez.Error.AlreadyConnected");
const QString kErrAlreadyExists = QStringLiteral("org.bluez.Error.AlreadyExists");

constexpr int kConnectTimeoutMs = 40000;
constexpr int kServicesTimeoutMs = 25000;

// The outcome of an async D-Bus call that a blocking wait may give up on. Shared
// by pointer so a late reply after the wait timed out lands somewhere valid.
struct CallState
{
    bool finished = false;
    QString errorName;
    QString errorMessage;
};

} // namespace

RingLink::RingLink(QObject *parent)
    : QObject(parent)
{
    registerBluezManagedObjectTypes();
}

RingLink::~RingLink()
{
    close(false);
}

QVariant RingLink::deviceProperty(const QString &name) const
{
    if (m_devicePath.isEmpty())
        return QVariant();
    QDBusInterface props(kBluezService, m_devicePath, kPropertiesInterface, QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> reply = props.call(QStringLiteral("Get"), kDeviceInterface, name);
    return reply.isValid() ? reply.value().variant() : QVariant();
}

bool RingLink::waitFor(const std::function<bool()> &done, int timeoutMs)
{
    if (done())
        return true;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    m_wake = [&]() {
        if (done())
            loop.quit();
    };
    timer.start(timeoutMs);
    loop.exec();
    m_wake = nullptr;
    return done();
}

QString RingLink::pair(const QString &devicePath, int timeoutMs)
{
    if (devicePath != m_devicePath) {
        close(false);
        m_devicePath = devicePath;
    }
    if (!m_deviceHooked) {
        QDBusConnection::systemBus().connect(
                kBluezService, m_devicePath, kPropertiesInterface, QStringLiteral("PropertiesChanged"), this,
                SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));
        m_deviceHooked = true;
    }

    // Trusted first: BlueZ then reconnects the ring on its own and does not ask
    // again before accepting its connections.
    QDBusInterface props(kBluezService, m_devicePath, kPropertiesInterface, QDBusConnection::systemBus());
    props.call(QStringLiteral("Set"), kDeviceInterface, QStringLiteral("Trusted"),
               QVariant::fromValue(QDBusVariant(true)));

    m_paired = deviceProperty(QStringLiteral("Paired")).toBool();
    if (m_paired)
        return QString();

    auto state = std::make_shared<CallState>();
    auto *watcher = new QDBusPendingCallWatcher(
            BluezCall::async(m_devicePath, kDeviceInterface, QStringLiteral("Pair"), {}, timeoutMs), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, state](QDBusPendingCallWatcher *w) {
        const QDBusPendingReply<> reply = *w;
        state->finished = true;
        if (reply.isError()) {
            state->errorName = reply.error().name();
            state->errorMessage = reply.error().message();
        }
        w->deleteLater();
        if (m_wake)
            m_wake();
    });

    if (!waitFor([state]() { return state->finished; }, timeoutMs))
        return tr("Pairing timed out. Is the ring close to the phone and not connected to another device?");

    if (!state->errorName.isEmpty() && state->errorName != kErrAlreadyExists) {
        if (state->errorName.endsWith(QLatin1String("AuthenticationFailed"))
            || state->errorName.endsWith(QLatin1String("AuthenticationRejected"))
            || state->errorName.endsWith(QLatin1String("AuthenticationCanceled")))
            return tr("The ring refused to pair (%1). A ring that is still set up with the Oura app "
                      "must be factory reset first.").arg(state->errorMessage);
        return tr("Pairing failed: %1\n\nIf this keeps happening, pair the ring once in Settings > Bluetooth, then set it up here.")
                .arg(state->errorMessage);
    }
    m_paired = true;
    return QString();
}

QString RingLink::open(const QString &devicePath)
{
    // Every open is a fresh session: close() stops notifications and drops the
    // hooks, so a repeated sync never inherits a doubled subscription.
    close(false);
    m_devicePath = devicePath;

    if (!m_deviceHooked) {
        QDBusConnection::systemBus().connect(
                kBluezService, m_devicePath, kPropertiesInterface, QStringLiteral("PropertiesChanged"), this,
                SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));
        m_deviceHooked = true;
    }

    m_connected = deviceProperty(QStringLiteral("Connected")).toBool();
    m_servicesResolved = deviceProperty(QStringLiteral("ServicesResolved")).toBool();

    if (!m_connected) {
        auto state = std::make_shared<CallState>();
        auto *watcher = new QDBusPendingCallWatcher(
                BluezCall::async(m_devicePath, kDeviceInterface, QStringLiteral("Connect"), {}, kConnectTimeoutMs), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, state](QDBusPendingCallWatcher *w) {
            const QDBusPendingReply<> reply = *w;
            state->finished = true;
            if (reply.isError()) {
                state->errorName = reply.error().name();
                state->errorMessage = reply.error().message();
            }
            w->deleteLater();
            if (m_wake)
                m_wake();
        });
        if (!waitFor([state]() { return state->finished; }, kConnectTimeoutMs))
            return tr("Could not reach the ring. Is it nearby, charged, and not connected to another phone?");
        // InProgress and AlreadyConnected mean "it is coming" and "it is there".
        if (!state->errorName.isEmpty() && state->errorName != kErrInProgress
            && state->errorName != kErrAlreadyConnected)
            return tr("Could not connect to the ring: %1").arg(state->errorMessage);
        m_connected = deviceProperty(QStringLiteral("Connected")).toBool();
        m_servicesResolved = deviceProperty(QStringLiteral("ServicesResolved")).toBool();
    }

    if (!waitFor([this]() { return m_servicesResolved; }, kServicesTimeoutMs))
        return tr("The ring connected but did not finish Bluetooth service discovery.");

    const QString found = discoverCharacteristics();
    if (!found.isEmpty())
        return found;

    // Subscribe. The hook must exist exactly once: connect() adds another hook
    // instead of recognising the one already there, and a doubled hook delivers
    // every notification twice.
    if (!m_notifyHooked) {
        QDBusConnection::systemBus().connect(
                kBluezService, m_notifyPath, kPropertiesInterface, QStringLiteral("PropertiesChanged"), this,
                SLOT(onNotifyPropertiesChanged(QString, QVariantMap, QStringList)));
        m_notifyHooked = true;
    }
    QDBusReply<void> reply(BluezCall::call(m_notifyPath, kCharInterface, QStringLiteral("StartNotify")));
    if (!reply.isValid()) {
        if (reply.error().name() == QLatin1String("org.bluez.Error.NotPermitted")
            || reply.error().message().contains(QLatin1String("nsufficient"), Qt::CaseInsensitive))
            return tr("The ring refused notifications because the link is not encrypted. Pair the ring first.");
        return tr("Could not subscribe to ring notifications: %1").arg(reply.error().message());
    }
    m_notifySubscribed = true;
    return QString();
}

QString RingLink::discoverCharacteristics()
{
    QDBusInterface manager(kBluezService, QStringLiteral("/"), kObjectManagerInterface,
                           QDBusConnection::systemBus());
    QDBusReply<BluezObjectMap> reply = manager.call(QStringLiteral("GetManagedObjects"));
    if (!reply.isValid())
        return tr("Could not list Bluetooth services: %1").arg(reply.error().message());

    const QString devicePrefix = m_devicePath + QLatin1Char('/');
    const BluezObjectMap objects = reply.value();

    QString servicePath;
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
        const QString path = it.key().path();
        if (!path.startsWith(devicePrefix) || !it.value().contains(kServiceInterface))
            continue;
        const QString uuid = it.value().value(kServiceInterface).value(QStringLiteral("UUID")).toString();
        if (uuid.compare(QLatin1String(Oura::kServiceUuid), Qt::CaseInsensitive) == 0) {
            servicePath = path;
            break;
        }
    }
    if (servicePath.isEmpty())
        return tr("This device does not offer the Oura ring service.");

    m_writePath.clear();
    m_notifyPath.clear();
    const QString servicePrefix = servicePath + QLatin1Char('/');
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
        const QString path = it.key().path();
        if (!path.startsWith(servicePrefix) || !it.value().contains(kCharInterface))
            continue;
        const QString uuid = it.value().value(kCharInterface).value(QStringLiteral("UUID")).toString();
        if (uuid.compare(QLatin1String(Oura::kWriteCharUuid), Qt::CaseInsensitive) == 0)
            m_writePath = path;
        else if (uuid.compare(QLatin1String(Oura::kNotifyCharUuid), Qt::CaseInsensitive) == 0)
            m_notifyPath = path;
    }
    if (m_writePath.isEmpty() || m_notifyPath.isEmpty())
        return tr("The ring service is missing its read or write characteristic.");
    return QString();
}

void RingLink::close(bool disconnectDevice)
{
    // Stopping notifications is not optional. Dropping our signal hook only
    // stops us listening; BlueZ still counts this connection as a notifying
    // client of the characteristic and emits PropertiesChanged once per such
    // client. A session that let go without StopNotify made the next one
    // receive every notification twice, which corrupts any reply that spans
    // several of them. Errors are ignored: "not notifying" is what we want.
    if (m_notifyHooked) {
        QDBusConnection::systemBus().disconnect(
                kBluezService, m_notifyPath, kPropertiesInterface, QStringLiteral("PropertiesChanged"), this,
                SLOT(onNotifyPropertiesChanged(QString, QVariantMap, QStringList)));
        m_notifyHooked = false;
    }
    if (m_notifySubscribed && !m_notifyPath.isEmpty()) {
        BluezCall::call(m_notifyPath, kCharInterface, QStringLiteral("StopNotify"));
    }
    m_notifySubscribed = false;

    if (m_deviceHooked) {
        QDBusConnection::systemBus().disconnect(
                kBluezService, m_devicePath, kPropertiesInterface, QStringLiteral("PropertiesChanged"), this,
                SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));
        m_deviceHooked = false;
    }
    if (disconnectDevice && !m_devicePath.isEmpty()) {
        BluezCall::async(m_devicePath, kDeviceInterface, QStringLiteral("Disconnect"));
    }

    m_writePath.clear();
    m_notifyPath.clear();
    m_devicePath.clear();
    m_connected = false;
    m_servicesResolved = false;
    m_frames.clear();
    m_collecting = false;
}

void RingLink::onDevicePropertiesChanged(const QString &interface, const QVariantMap &changed,
                                         const QStringList &invalidated)
{
    Q_UNUSED(invalidated);
    if (interface != kDeviceInterface)
        return;
    if (changed.contains(QStringLiteral("Connected"))) {
        m_connected = changed.value(QStringLiteral("Connected")).toBool();
        if (!m_connected) {
            m_servicesResolved = false;
            if (m_notifySubscribed)
                emit linkLost();
        }
    }
    if (changed.contains(QStringLiteral("ServicesResolved")))
        m_servicesResolved = changed.value(QStringLiteral("ServicesResolved")).toBool();
    if (changed.contains(QStringLiteral("Paired")))
        m_paired = changed.value(QStringLiteral("Paired")).toBool();
    if (m_wake)
        m_wake();
}

void RingLink::onNotifyPropertiesChanged(const QString &interface, const QVariantMap &changed,
                                         const QStringList &invalidated)
{
    Q_UNUSED(invalidated);
    if (interface != kCharInterface || !changed.contains(QStringLiteral("Value")))
        return;
    if (!m_collecting)
        return; // unsolicited, or the answer to a request we already gave up on
    m_frames.append(changed.value(QStringLiteral("Value")).toByteArray());
    if (m_wake)
        m_wake();
}

bool RingLink::transact(const Oura::Bytes &request, const std::function<bool(const Oura::Packet &)> &terminal,
                        int quietMs, std::vector<Oura::Packet> &out, std::string &error)
{
    if (!m_notifySubscribed || !m_connected) {
        error = tr("The ring is not connected").toStdString();
        return false;
    }

    m_frames.clear();
    m_collecting = true;

    const QByteArray data(reinterpret_cast<const char *>(request.data()), static_cast<int>(request.size()));
    QDBusReply<void> written(BluezCall::call(m_writePath, kCharInterface, QStringLiteral("WriteValue"),
                                             { data, QVariantMap() }));
    if (!written.isValid()) {
        m_collecting = false;
        error = tr("Could not write to the ring: %1").arg(written.error().message()).toStdString();
        return false;
    }

    int consumed = 0;
    bool terminalSeen = false;
    // Folds in frames that arrived since the last look. Returns true when the
    // request is complete: its known reply came, or the link went away.
    const auto process = [&]() {
        while (consumed < m_frames.size()) {
            const QByteArray &frame = m_frames.at(consumed++);
            const Oura::Bytes bytes(frame.begin(), frame.end());
            for (Oura::Packet &p : Oura::parsePackets(bytes)) {
                if (terminal(p))
                    terminalSeen = true;
                out.push_back(std::move(p));
            }
        }
        return terminalSeen || !m_connected;
    };

    // A reply can already be here: it may have arrived while WriteValue blocked.
    if (!process()) {
        QEventLoop loop;
        QTimer quiet;
        quiet.setSingleShot(true);
        connect(&quiet, &QTimer::timeout, &loop, &QEventLoop::quit);
        int seenBefore = consumed;
        m_wake = [&]() {
            if (process())
                loop.quit();
            else if (consumed != seenBefore) {
                // Every frame that arrives restarts the quiet window: a long
                // reply is not "quiet" while it is still coming in.
                seenBefore = consumed;
                quiet.start(quietMs);
            }
        };
        quiet.start(quietMs);
        loop.exec();
        m_wake = nullptr;
    }
    m_collecting = false;

    if (!m_connected) {
        error = tr("Bluetooth link to the ring was lost").toStdString();
        return false;
    }
    return true;
}
