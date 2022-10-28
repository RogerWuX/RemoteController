#include "Logger.h"
#include "spdlog/async.h"

const LogManager &LogManager::Ins()
{
    static LogManager* pkIns = nullptr;
    if(!pkIns)
        pkIns = new LogManager();
    return *pkIns;
}

LogManager::LogManager()
    :m_pkRtcLogger(spdlog::basic_logger_mt<spdlog::async_factory>("rtc","logs/rtc.log")),
     m_pkMainLogger(spdlog::basic_logger_mt<spdlog::async_factory>("main","logs/main.log"))
{
    m_pkRtcLogger->flush_on(spdlog::level::warn);
    m_pkMainLogger->flush_on(spdlog::level::warn);
}
