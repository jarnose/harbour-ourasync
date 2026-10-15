#include "exportimport.h"

#include "zip.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <functional>

namespace {

bool wanted(const QString &name)
{
    const QString lower = QFileInfo(name).fileName().toLower();
    return lower == QLatin1String("heartrate.csv") || !Oura::kindForCsvName(lower.toStdString()).empty();
}

} // namespace

ExportReadResult readOuraExport(const QString &path, const std::function<void(const QString &)> &progress)
{
    ExportReadResult result;
    const QFileInfo info(path);
    if (!info.exists()) {
        result.error = QStringLiteral("The file does not exist");
        return result;
    }

    const auto readCsv = [&](const QString &name, const QByteArray &bytes) {
        progress(QObject::tr("Reading %1…").arg(QFileInfo(name).fileName()));
        Oura::addCsvFile(result.data, name.toStdString(), std::string(bytes.constData(), static_cast<size_t>(bytes.size())));
    };

    if (info.isDir()) {
        // The export unzipped by hand: look through it for the CSV files.
        QDir dir(path);
        QStringList stack { path };
        while (!stack.isEmpty()) {
            QDir d(stack.takeLast());
            for (const QFileInfo &entry : d.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
                if (entry.isDir()) {
                    stack.append(entry.absoluteFilePath());
                } else if (wanted(entry.fileName())) {
                    QFile f(entry.absoluteFilePath());
                    if (f.open(QIODevice::ReadOnly))
                        readCsv(entry.fileName(), f.readAll());
                }
            }
        }
    } else if (info.suffix().compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            result.error = QObject::tr("Could not open the file");
            return result;
        }
        readCsv(info.fileName(), f.readAll());
    } else {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            result.error = QObject::tr("Could not open the file");
            return result;
        }
        progress(QObject::tr("Opening the archive…"));
        const QByteArray bytes = f.readAll();
        const std::string archive(bytes.constData(), static_cast<size_t>(bytes.size()));
        std::vector<Oura::ZipEntry> entries;
        std::string error;
        if (!Oura::listZip(archive, entries, error)) {
            result.error = QString::fromStdString(error);
            return result;
        }
        for (const Oura::ZipEntry &entry : entries) {
            if (!wanted(QString::fromStdString(entry.name)))
                continue;
            std::string text;
            if (!Oura::extractZipEntry(archive, entry, text, error)) {
                result.error = QString::fromStdString(error);
                return result;
            }
            progress(QObject::tr("Reading %1…").arg(QFileInfo(QString::fromStdString(entry.name)).fileName()));
            Oura::addCsvFile(result.data, entry.name, text);
        }
    }

    if (result.data.docs.empty() && result.data.hr.empty()) {
        result.error = QObject::tr("No Oura data files were found. The export is a ZIP of CSV files such as "
                                   "dailysleep.csv, from membership.ouraring.com/data-export.");
        return result;
    }
    result.ok = true;
    return result;
}
