#pragma once

#include "bluezmanagedobjects.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

// Finds Oura rings through BlueZ's own D-Bus API (org.bluez.*), not Qt
// Bluetooth: Sailfish OS 5.1's SDK target has no pkgconfig(Qt5Bluetooth), so
// QLowEnergyController is not available. Same approach as harbour-suuntosync,
// whose BlueZ glue this follows.
//
// A ring is recognised by its advertised service UUID or by "oura" in its name.
// A factory-reset ring advertises a name like "Oura <serial digits>"; once it
// has an app key it advertises as "Oura Ring Gen3" or similar, and a bonded one
// often advertises no name at all, so the UUID is the more dependable test.
class RingScanner : public QObject
{
    Q_OBJECT
public:
    struct Device
    {
        QString objectPath;
        QString address;
        QString name;
        bool paired = false;
        bool connected = false;
        bool trusted = false;
        int rssi = 0; // dBm, 0 when BlueZ has no reading
    };

    explicit RingScanner(QObject *parent = nullptr);

    // Re-reads every device BlueZ knows and emits deviceUpdated() for the rings.
    void refresh();

    // Scanning also polls refresh() every couple of seconds: RSSI and the
    // paired/connected flags change without a new object appearing.
    void startDiscovery();
    void stopDiscovery();
    bool discovering() const { return m_discovering; }

    // Object path of the BlueZ device with this MAC address, or an empty string.
    QString devicePathForAddress(const QString &address);

    // Makes BlueZ forget the device and its bond.
    bool removeDevice(const QString &objectPath, QString *error = nullptr);

    static bool looksLikeOura(const QVariantMap &deviceProperties);

signals:
    void deviceUpdated(const RingScanner::Device &device);
    void errorOccurred(const QString &message);

private slots:
    void onInterfacesAdded(const QDBusObjectPath &path, const BluezInterfaceMap &interfaces);

private:
    QString adapterPath();
    void emitIfRing(const QString &objectPath, const BluezInterfaceMap &interfaces);

    QString m_adapterPath;
    QTimer m_poll;
    bool m_discovering = false;
};

Q_DECLARE_METATYPE(RingScanner::Device)
