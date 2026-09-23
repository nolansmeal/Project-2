#pragma once

#include <string>

using namespace std;

enum class Role {
    System,
    User,
    Assistant
};

class Message {
public:

    // Default constructor
    Message() {
        role_ = Role::System;
        content_ = "";
    }

    // Constructor
    Message(Role role, string content) {
        role_ = role;
        content_ = content;
    }

    // Returns the role
    Role role() const noexcept {
        return role_;
    }

    // Returns the message content
    const string& content() const noexcept {
        return content_;
    }

private:
    Role role_;
    string content_;
};