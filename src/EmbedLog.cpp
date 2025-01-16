/*
 * EmbedLog - A Minimal Logging Library for Embedded Systems
 *
 * Copyright (C) 2023 Joe Inman
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License as published by
 * the Open Source Initiative.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * MIT License for more details.
 *
 * You should have received a copy of the MIT License along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 *
 * Author: Joe Inman
 * Email: joe.inman8@gmail.com
 * Version: 1.0
 *
 * Description:
 * EmbedLog is designed to provide a lightweight and flexible logging system 
 * for embedded environments. It supports multiple log levels, allows 
 * user-defined output mechanisms, and includes timestamping based on 
 * microsecond-resolution functions.
 *
 */

#include "EmbedLog/EmbedLog.hpp"

#include <iomanip>

namespace EmbedLog
{

uint64_t unique_id(std::string file, int line)
{
    return std::hash<std::string>{}(file + std::to_string(line));
}

EmbedLog::EmbedLog(
    OpenFunction open_func,
    CloseFunction close_func,
    PrintFunction print_func,
    MicrosecondFunction microsecond_func,
    std::string name,
    std::string format)
    : m_open_func(open_func),
      m_close_func(close_func),
      m_print_func(print_func),
      m_microsecond_func(microsecond_func),
      m_name(name),
      m_format(format)
{
}

EmbedLog::~EmbedLog()
{
    if (m_is_open)
        m_close_func();
}

bool EmbedLog::open()
{
    if (!m_is_open)
        m_is_open = m_open_func();
    return m_is_open;
}

bool EmbedLog::close()
{
    bool result = m_close_func();
    m_is_open = !result;
    return result;
}

void EmbedLog::log(LogLevel level, const std::string &format, ...)
{
    if (!m_is_open)
        return;

    if (level < m_log_level)
        return;

    va_list args;
    va_start(args, format);

    // First pass to get the required buffer size
    int size = vsnprintf(nullptr, 0, format.c_str(), args);
    va_end(args);

    if (size < 0)
        return; // Handle error in formatting

    // Allocate a buffer of the required size
    std::vector<char> buffer(
        static_cast<std::vector<char>::size_type>(size) + 1); // +1 for the null terminator

    va_start(args, format);
    vsnprintf(buffer.data(), buffer.size(), format.c_str(), args);
    va_end(args);

    print(level, buffer.data());
}

void EmbedLog::log_throttled(
    size_t throttle_id, uint32_t throttle_ms, LogLevel level, const std::string &format, ...)
{
    if (!m_is_open)
        return;

    if (level < m_log_level)
        return;

    auto now = m_microsecond_func();
    auto last = m_throttle_map[throttle_id];
    if (now - last > throttle_ms * 1000)
    {
        va_list args;
        va_start(args, format);

        // First pass to get the required buffer size
        int size = vsnprintf(nullptr, 0, format.c_str(), args);
        va_end(args);

        if (size < 0)
            return; // Handle error in formatting

        // Allocate a buffer of the required size
        std::vector<char> buffer(
            static_cast<std::vector<char>::size_type>(size) + 1); // +1 for the null terminator

        va_start(args, format);
        vsnprintf(buffer.data(), buffer.size(), format.c_str(), args);
        va_end(args);

        print(level, buffer.data()); // Use buffer.data() for the pointer to the char array
        m_throttle_map[throttle_id] = now;
    }
}

void EmbedLog::print(LogLevel level, const std::string &message)
{
    uint64_t microseconds = m_microsecond_func();
    uint64_t totalSeconds = microseconds / 1000000;
    uint64_t remainingMicroseconds = microseconds % 1000000;

    uint64_t hours = totalSeconds / 3600;
    uint64_t minutes = (totalSeconds % 3600) / 60;
    uint64_t seconds = totalSeconds % 60;

    std::stringstream result;
    result << "\u001b[1m";
    for (size_t i = 0; i < m_format.size(); ++i)
    {
        if (m_format[i] == '%')
        {
            ++i; // Skip the '%' and check the next character
            switch (m_format[i])
            {
            case 'N':
                result << m_name; // Name
                break;
            case 'L':
                result << getLogLevelString(level); // Level
                break;
            case 'D':
                result << std::setfill('0') << std::setw(2) << (hours / 24); // Days
                break;
            case 'H':
                result << std::setfill('0') << std::setw(2) << (hours % 24); // Hours
                break;
            case 'M':
                result << std::setfill('0') << std::setw(2) << minutes; // Minutes
                break;
            case 'S':
                result << std::setfill('0') << std::setw(2) << seconds; // Seconds
                break;
            case 'U':
                result << std::setfill('0') << std::setw(6) << remainingMicroseconds; // Microseconds
                break;
            default:
                result << '%' << m_format[i]; // Unknown
                break;
            }
        }
        else
        {
            result << m_format[i]; // Normal character
        }
    }

    result << " " << "\033[0m" << message << "\n";
    m_print_func(result.str());
}

void EmbedLog::setLogLevel(LogLevel level) { m_log_level = level; }

std::string EmbedLog::getLogLevelString(LogLevel level)
{
    switch (level)
    {
    case INFO:
        return "\033[1;92mINFO\u001b[0m\u001b[1m";
    case WARNING:
        return "\033[1;93mWARNING\u001b[0m\u001b[1m";
    case ERROR:
        return "\033[1;91mERROR\u001b[0m\u001b[1m";
    case DEBUG:
        return "\033[1;94mDEBUG\u001b[0m\u001b[1m";
    case NONE:
        return "NONE";
    default:
        return "UNKNOWN";
    }
}

} // namespace EmbedLog
