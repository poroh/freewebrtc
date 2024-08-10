//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Pair State (RFC8445)
//

#include "ice/pair/ice_pair_state.hpp"
#include "ice/pair/ice_pair_error.hpp"

namespace freewebrtc::ice::pair {

Event::Event(Value v)
    : m_value(v)
{}

Event Event::unfreeze() noexcept {
    return Event{Value::unfreeze};
}

Event Event::perform() noexcept {
    return Event{Value::perform};
}

Event Event::failure() noexcept {
    return Event{Value::failure};
}

Event Event::success() noexcept {
    return Event{Value::success};
}

std::string Event::to_string() const {
    switch (m_value) {
    case Value::unfreeze: return "unfreeze";
    case Value::perform:  return "perform";
    case Value::failure:  return "failure";
    case Value::success:  return "success";
    }
    return "unknown";
}

State::State(Value v)
    : m_value(v)
{}

State State::waiting() noexcept {
    return State(Value::waiting);
}

State State::in_progress() noexcept {
    return State(Value::in_progress);
}

State State::succeeded() noexcept {
    return State(Value::succeeded);
}

State State::failed() noexcept {
    return State(Value::failed);
}

State State::frozen() noexcept {
    return State(Value::frozen);
}


std::string State::to_string() const {
    switch (m_value) {
    case Value::frozen:      return "Frozen";
    case Value::waiting:     return "Waiting";
    case Value::in_progress: return "In-Progress";
    case Value::succeeded:   return "Succeeded";
    case Value::failed:      return "Failed";
    }
    return "unknown";
}

Result<State> State::transition(Event ev) const noexcept {
    // RFC8445:
    // 6.1.2.6.  Computing Candidate Pair States
    switch (m_value) {
    case Value::frozen:
        if (ev == Event::unfreeze()) {
            return State(Value::waiting);
        }
        break;
    case Value::waiting:
        if (ev == Event::perform()) {
            return State(Value::in_progress);
        }
        break;
    case Value::in_progress:
        if (ev == Event::success()) {
            return State(Value::succeeded);
        } else if (ev == Event::failure()) {
            return State(Value::failed);
        }
        break;
    case Value::succeeded:
    case Value::failed:
        break;
    }
    return Result<State>(make_error_code(Error::ice_pair_state_unexpected_event))
        .add_context("state: " + to_string() + "; event: " + ev.to_string());
}

}
