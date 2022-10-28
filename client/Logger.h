#pragma once

#define RTC_LOGGER LogManager::Ins().m_pkRtcLogger
#define MAIN_LOGGER LogManager::Ins().m_pkMainLogger

class LogManager
{
public:
    static LogManager const& Ins();
public:
    const std::shared_ptr<spdlog::logger> m_pkRtcLogger;
    const std::shared_ptr<spdlog::logger> m_pkMainLogger;
private:
    LogManager();
};
