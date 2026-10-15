#pragma once

#include "cloud/clouddoc.h"
#include "daysummary.h"

#include <vector>

namespace Oura {

// Folds Oura's cloud documents into a summary built from the ring.
//
//  * Oura's own scores (sleep, readiness, activity) and their contributors are
//    attached to the matching night or day, next to this app's own estimate.
//  * Nights and days the ring has not been read for (everything before the first
//    sync here: a ring keeps only a few weeks) are added from the cloud, with
//    stages, heart rate and HRV where the cloud has them, and marked fromCloud.
//  * Where ring and cloud cover the same time, the ring's own data is kept.
void mergeCloud(Summary &summary, const std::vector<CloudDoc> &docs, const std::vector<CloudHrPoint> &hr,
                const AnalysisOptions &options);

} // namespace Oura
