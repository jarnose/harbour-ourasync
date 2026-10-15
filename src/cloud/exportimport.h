#pragma once

#include "clouddoc.h"

#include <QString>
#include <QStringList>

#include <functional>

// Reads an Oura data export: the ZIP as downloaded, a folder holding its CSV
// files, or a single CSV file. Done off the interface thread (zip inflation and a
// year of heart rate take seconds); nothing here touches Qt objects that belong
// to another thread.
struct ExportReadResult
{
    bool ok = false;
    QString error;
    Oura::CloudImport data;
};

// Calls `progress` with a short text now and then, from the calling thread.
ExportReadResult readOuraExport(const QString &path, const std::function<void(const QString &)> &progress);
