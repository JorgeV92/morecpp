#pragma once

#include <boost/algorithm/string.hpp>

#include <string>

namespace boost_chat {

inline constexpr std::size_t max_line_size = 4096;

inline std::string trim_wire_line(std::string line) {
    boost::algorithm::trim_right_if(line, boost::algorithm::is_any_of("\r\n"));
    return line;
}

inline std::string to_wire_line(std::string line) {
    line = trim_wire_line(std::move(line));
    line.push_back('\n');
    return line;
}

inline std::string server_line(std::string line) {
    return to_wire_line("[server] " + std::move(line));
}

}  // namespace boost_chat
