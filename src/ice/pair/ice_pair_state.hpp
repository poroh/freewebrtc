//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Pair State (RFC8445)
// 6.1.2.6. Computing Candidate Pair States
//

#pragma once

#include "util/util_result.hpp"

namespace freewebrtc::ice::pair {

class Event {
public:
    enum class Value {
        unfreeze,
        perform,
        failure,
        success
    };
    static Event unfreeze() noexcept;
    static Event perform() noexcept;
    static Event failure() noexcept;
    static Event success() noexcept;

    Value value() const noexcept;
    std::string to_string() const;

    bool operator==(const Event&) const noexcept = default;
private:
    Event(Value);
    Value m_value;
};

class State {
public:
    enum class Value {
        waiting,
        in_progress,
        succeeded,
        failed,
        frozen
    };
    static State waiting() noexcept;
    static State in_progress() noexcept;
    static State succeeded() noexcept;
    static State failed() noexcept;
    static State frozen() noexcept;

    Result<State> transition(Event) const noexcept;

    Value value() const noexcept;
    std::string to_string() const;

    bool operator==(const State&) const noexcept = default;

private:
    State(Value);
    Value m_value;
};

//
// implementation
//
inline Event::Value Event::value() const noexcept {
    return m_value;
}

inline State::Value State::value() const noexcept {
    return m_value;
}

}
