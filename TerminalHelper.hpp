#pragma once

#include <imterm/terminal.hpp>
#include <imterm/terminal_helpers.hpp>

class TerminalHelper : public ImTerm::basic_terminal_helper<TerminalHelper, void> {
public:
    static std::vector<std::string> no_completion(argument_type& arg) {
        return {};
    }

    static void clear(argument_type& arg) {
        arg.term.clear();
    }

    static void echo(argument_type& arg) {
        if (arg.command_line.size() < 2) {
            return;
        }
        std::string str = std::move(arg.command_line[1]);
        for (auto it = std::next(arg.command_line.begin(), 2); it != arg.command_line.end(); ++it) {
            str += " " + std::move(*it);
        }
        ImTerm::message msg;
        msg.value = std::move(str);
        msg.color_beg = msg.color_end = 0;
        arg.term.add_message(std::move(msg));
    }

    TerminalHelper() {
        add_command_({ "clear", "clear the screen", clear, no_completion });
        add_command_({ "echo", "echoes your text", echo, no_completion });
    }
};