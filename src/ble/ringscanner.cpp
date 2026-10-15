#include "ringscanner.h"

#include "ring/protocol.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>

namespace {

const QString kBluezService = QStringLiteral("org.bluez");
const QString kObjectManagerInterface = QStringLiteral("org.freedesktop.DBus.ObjectManager");
const QString kAdapterInterface = QStringLiteral("org.bluez.Adapter1");
const QString kDeviceInterface = QStringLiteral("org.bluez.Device1");

} // namespace

RingScanner::RingScanner(QObject *parent)
    : QObject(parent)
{
    registerBluezManagedObjectTypes();
    qRegisterMetaType<RingScanner::Device>();

    QDBusConnection::systemBus().connect(
            kBluezService, QString(), kObjectManagerInterface, QStringLiteral("InterfacesAdded"), this,
            SLOT(onInterfacesAdded(QDBusObjectPath, BluezInterfaceMap)));

    m_poll.setInterval(2000);
    connect(&m_poll, &QTimer::timeout, this, &RingScanner::refresh);
}

bool RingScanner::looksLikeOura(const QVariantMap &props)
{
    const QStringList uuids = props.value(QStringLiteral("UUIDs")).toStringList();
    for (const QString &uuid : uuids)
        if (uuid.compare(QLatin1String(Oura::kServiceUuid), Qt::CaseInsensitive) == 0)
            return true;
    const QString name = props.value(QStringLiteral("Alias"), props.value(QStringLiteral("Name"))).toString();
    return name.contains(QLatin1String("oura"), Qt::CaseInsensitive);
}

QString RingScanner::adapterPath()
{
    if (!m_adapterPath.isEmpty())
        return m_adapterPath;

    QDBusInterface manager(kBluezService, QStringLiteral("/"), kObjectManagerInterface,
                           QDBusConnection::systemBus());
    QDBusReply<BluezObjectMap> reply = manager.call(QStringLiteral("GetManagedObjects"));
    if (!reply.isValid()) {
        emit errorOccurred(tr("Could not reach Bluetooth (org.bluez): %1").arg(reply.error().message()));
        return QString();
    }
    const BluezObjectMap objects = reply.value();
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
        if (it.value().contains(kAdapterInterface)) {
            m_adapterPath = it.key().path();
            break;
        }
    }
    if (m_adapterPath.isEmpty())
        emit errorOccurred(tr("No Bluetooth adapter found"));
    return m_adapterPath;
}

void RingScanner::emitIfRing(const QString &objectPath, const BluezInterfaceMap &interfaces)
{
    if (!interfaces.contains(kDeviceInterface))
        return;
    const QVariantMap props = interfaces.value(kDeviceInterface);
    if (!looksLikeOura(props))
        return;

    Device d;
    d.objectPath = objectPath;
    d.address = props.value(QStringLiteral("Address")).toString();
    d.name = props.value(QStringLiteral("Alias"), props.value(QStringLiteral("Name"), d.address)).toString();
    d.paired = props.value(QStringLiteral("Paired")).toBool();
    d.connected = props.value(QStringLiteral("Connected")).toBool();
    d.trusted = props.value(QStringLiteral("Trusted")).toBool();
    d.rssi = props.value(QStringLiteral("RSSI")).toInt();
    emit deviceUpdated(d);
}

void RingScanner::refresh()
{
    QDBusInterface manager(kBluezService, QStringLiteral("/"), kObjectManagerInterface,
                           QDBusConnection::systemBus());
    QDBusReply<BluezObjectMap> reply = manager.call(QStringLiteral("GetManagedObjects"));
    if (!reply.isValid()) {
        emit errorOccurred(tr("Could not list Bluetooth devices: %1").arg(reply.error().message()));
        return;
    }
    const BluezObjectMap objects = reply.value();
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it)
        emitIfRing(it.key().path(), it.value());
}

void RingScanner::onInterfacesAdded(const QDBusObjectPath &path, const BluezInterfaceMap &interfaces)
{
    emitIfRing(path.path(), interfaces);
}

void RingScanner::startDiscovery()
{
    const QString path = adapterPath();
    if (path.isEmpty())
        return;
    m_discovering = true;
    m_poll.start();

    QDBusInterface adapter(kBluezService, path, kAdapterInterface, QDBusConnection::systemBus());
    auto *watcher = new QDBusPendingCallWatcher(adapter.asyncCall(QStringLiteral("StartDiscovery")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        const QDBusPendingReply<> reply = *w;
        // InProgress means someone else (Settings) is already scanning, which
        // serves us just as well.
        if (reply.isError() && reply.error().name() != QLatin1String("org.bluez.Error.InProgress"))
            emit errorOccurred(tr("Could not start scanning: %1").arg(reply.error().message()));
        w->deleteLater();
    });
}

void RingScanner::stopDiscovery()
{
    m_poll.stop();
    if (!m_discovering)
        return;
    m_discovering = false;
    const QString path = adapterPath();
    if (path.isEmpty())
        return;
    QDBusInterface adapter(kBluezService, path, kAdapterInterface, QDBusConnection::systemBus());
    adapter.asyncCall(QStringLiteral("StopDiscovery"));
}

QString RingScanner::devicePathForAddress(const QString &address)
{
    QDBusInterface manager(kBluezService, QStringLiteral("/"), kObjectManagerInterface,
                           QDBusConnection::systemBus());
    QDBusReply<BluezObjectMap> reply = manager.call(QStringLiteral("GetManagedObjects"));
    if (!reply.isValid())
        return QString();
    const BluezObjectMap objects = reply.value();
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
        const QVariantMap props = it.value().value(kDeviceInterface);
        if (!props.isEmpty()
            && props.value(QStringLiteral("Address")).toString().compare(address, Qt::CaseInsensitive) == 0)
            return it.key().path();
    }
    return QString();
}

bool RingScanner::removeDevice(const QString &objectPath, QString *error)
{
    const QString path = adapterPath();
    if (path.isEmpty()) {
        if (error)
            *error = tr("No Bluetooth adapter found");
        return false;
    }
    QDBusInterface adapter(kBluezService, path, kAdapterInterface, QDBusConnection::systemBus());
    QDBusReply<void> reply = adapter.call(QStringLiteral("RemoveDevice"), QVariant::fromValue(QDBusObjectPath(objectPath)));
    if (!reply.isValid()) {
        if (error)
            *error = reply.error().message();
        return false;
    }
    return true;
}
