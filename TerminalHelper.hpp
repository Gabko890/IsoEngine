#pragma once

#include <imterm/terminal.hpp>
#include <imterm/terminal_helpers.hpp>

#include "Scene.hpp"
#include "Utils.hpp"

class TerminalHelper : public ImTerm::basic_terminal_helper<TerminalHelper, void> {
public:
    static Scene* scene;

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

    static void addobject(argument_type& arg) {
        ImTerm::message msg;
        if (arg.command_line.size() < 3) {
            msg.value = std::move("Syntax Error! \nUsage: addobject <id/name> <path_to_glb>");
        }
        else if (scene != NULL && scene->AddObject(arg.command_line[1], arg.command_line[2].c_str())) {
            msg.value = std::move("Object added succesfully!");
        }
        else {
            msg.value = std::move("Failed to add object!");
        }

        msg.color_beg = msg.color_end = 0;
        arg.term.add_message(std::move(msg));
    }

    static void rmobject(argument_type& arg) {
        ImTerm::message msg;
        if (arg.command_line.size() < 2) {
            msg.value = std::move("Syntax Error! \nUsage: rmobject <id/name>");
        }
        else if (scene != NULL && scene->RemoveObject(arg.command_line[1])) {
            msg.value = std::move("Object removed succesfully!");
        }
        else {
            msg.value = std::move("Failed to remove object!");
        }

        msg.color_beg = msg.color_end = 0;
        arg.term.add_message(std::move(msg));
    }

    static void bgcolor(argument_type& arg) {
        ImTerm::message msg;
        if (arg.command_line.size() < 4) {
            msg.value = std::move("Syntax Error! \nUsage: bgcolor <r> <g> <b>");
        }
        else {
            int r = atoi(arg.command_line[1].c_str());
            int g = atoi(arg.command_line[2].c_str());
            int b = atoi(arg.command_line[3].c_str());

            if (r > 255 || g > 255 || b > 255) {
                msg.value = std::move("Syntax Error! \nMaximal value is 255!");
            }

            else if (r < 0 || g < 0 || b < 0) {
                msg.value = std::move("Syntax Error! \nMinimal value is 0!");
            }

            else scene->SetBGColor(r, g, b);
        }
        
        msg.color_beg = msg.color_end = 0;
        arg.term.add_message(std::move(msg));
    }

    static void alias(argument_type& arg) {
        ImTerm::message msg;
        if (arg.command_line.size() < 3) {
            msg.value = std::move("Syntax Error! \nUsage: alias <key> <value>");
        }
        else {
            scene->AddPathAlias(arg.command_line[1], arg.command_line[2]);
            msg.value = std::move("Alias added succesfully!");
        }

        msg.color_beg = msg.color_end = 0;
        arg.term.add_message(std::move(msg));
    }

    TerminalHelper() {
        add_command_({ "clear", "clear the screen", clear, no_completion });
        add_command_({ "echo", "echoes your text", echo, no_completion });
        
        add_command_({ "addobject", "adds object to scene", addobject, no_completion });
        add_command_({ "rmobject", "removes object from scene", rmobject, no_completion });

        add_command_({ "bgcolor", "change color of renderer background", bgcolor, no_completion });
        add_command_({ "alias", "adds path alias", alias, no_completion });
    }
};

Scene* TerminalHelper::scene = NULL;