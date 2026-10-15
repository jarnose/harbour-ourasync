#pragma once

#include "ring/session.h"

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>

// The ring's GATT link over BlueZ's D-Bus API, as a ring::Transport.
//
// Everything that waits is a *blocking call that keeps the event loop
// running*: a nested QEventLoop, not a thread. That keeps the sync procedure a
// plain top-to-bottom function (connect, authenticate, drain) instead of a
// state machine scattered across callbacks, and the user interface stays alive
// because the nested loop still services it. The price is re-entrancy, which
// AppController pays by refusing a second operation while one is running.
//
// Each method that can fail returns an empty QString on success and a message a
// person can read on failure.
class RingLink : public QObject, public Oura::Transport
{
    Q_OBJECT
public:
    // True when the last request was written to the ring, whatever came back. A reset makes the
    // ring drop the link at once, which is an answer of a kind.
    bool lastWriteSucceeded() const { return m_lastWriteOk; }
    bool connectedNow() const { return m_connected; }
    explicit RingLink(QObject *parent = nullptr);
    ~RingLink() override;

    // Bonds with the device (Device1.Pair), which a factory-reset ring requires
    // before it will accept a single GATT write. A no-op for a device that is
    // already paired. Marks the device trusted so BlueZ reconnects it.
    QString pair(const QString &devicePath, int timeoutMs = 60000);

    // Connects, waits for GATT services, finds the ring's two characteristics
    // and starts notifications.
    QString open(const QString &devicePath);

    // Stops notifications (which is not optional, see close()) and optionally
    // drops the connection.
    void close(bool disconnectDevice);

    bool isOpen() const { return m_notifySubscribed; }
    QString devicePath() const { return m_devicePath; }

    bool transact(const Oura::Bytes &request, const std::function<bool(const Oura::Packet &)> &terminal,
                  int quietMs, std::vector<Oura::Packet> &out, std::string &error) override;

signals:
    void linkLost();

private slots:
    void onDevicePropertiesChanged(const QString &interface, const QVariantMap &changed,
                                   const QStringList &invalidated);
    void onNotifyPropertiesChanged(const QString &interface, const QVariantMap &changed,
                                   const QStringList &invalidated);

private:
    QString discoverCharacteristics();
    QVariant deviceProperty(const QString &name) const;
    // Spins the event loop until `done` is set, the timeout passes, or the
    // device disconnects. Returns false on timeout.
    bool waitFor(const std::function<bool()> &done, int timeoutMs);

    QString m_devicePath;
    QString m_writePath;
    QString m_notifyPath;
    bool m_deviceHooked = false;
    bool m_notifyHooked = false;
    bool m_notifySubscribed = false;
    bool m_servicesResolved = false;
    bool m_connected = false;
    bool m_paired = false;

    // The transaction in flight: frames land here, and `m_wake` ends the wait.
    bool m_collecting = false;
    bool m_lastWriteOk = false;
    QList<QByteArray> m_frames;
    std::function<void()> m_wake;
};
