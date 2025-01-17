// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef O2_FRAMEWORK_DATAPROCESSINGSTATS_H_
#define O2_FRAMEWORK_DATAPROCESSINGSTATS_H_

#include <atomic>
#include <cstdint>

namespace o2::framework
{
/// Helper struct to hold statistics about the data processing happening.
struct DataProcessingStats {
  constexpr static ServiceKind service_kind = ServiceKind::Global;
  // If we have more than this, we probably have other issues in any case
  constexpr static int MAX_RELAYER_STATES = 4096;
  // We use this to keep track of the latency of the first message we get for a given input record
  // and of the last one.
  struct InputLatency {
    int minLatency = 0;
    int maxLatency = 0;
  };
  std::atomic<int> errorCount = 0;
  std::atomic<int> exceptionCount = 0;
  std::atomic<int> pendingInputs = 0;
  std::atomic<int> incomplete = 0;
  std::atomic<int> inputParts = 0;
  std::atomic<int> lastElapsedTimeMs = 0;
  std::atomic<int> lastProcessedSize = 0;
  std::atomic<int> totalProcessedSize = 0;
  std::atomic<int> totalSigusr1 = 0;
  std::atomic<int> consumedTimeframes = 0;
  std::atomic<uint64_t> availableManagedShm = 0; /// Available shared memory in bytes.

  std::atomic<uint64_t> lastSlowMetricSentTimestamp = 0; /// The timestamp of the last time we sent slow metrics
  std::atomic<uint64_t> lastVerySlowMetricSentTimestamp = 0; /// The timestamp of the last time we sent very slow metrics
  std::atomic<uint64_t> lastMetricFlushedTimestamp = 0;  /// The timestamp of the last time we actually flushed metrics
  std::atomic<uint64_t> beginIterationTimestamp = 0;     /// The timestamp of when the current ConditionalRun was started

  std::atomic<uint64_t> performedComputations = 0;             // The number of computations which have completed so far
  std::atomic<uint64_t> lastReportedPerformedComputations = 0; // The number of computations which have completed until lastSlowMetricSentTimestamp

  std::vector<uint64_t> channelBytesIn;  // How many incoming bytes have gone through the channels
  std::vector<uint64_t> channelBytesOut; // How many outgoing bytes have gone through the channels

  InputLatency lastLatency = {0, 0};

  std::atomic<int> relayerState[MAX_RELAYER_STATES];
  std::atomic<size_t> statesSize;
};

} // namespace o2::framework

#endif // O2_FRAMEWORK_DATAPROCESSINGSTATS_H_
