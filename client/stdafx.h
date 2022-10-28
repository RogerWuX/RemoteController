#define WEBRTC_POSIX
#define ENCRYPT_MATCH 1

#include <memory>

#include "../protocol/signalling.pb.h"
#include "../protocol/p2p.pb.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
