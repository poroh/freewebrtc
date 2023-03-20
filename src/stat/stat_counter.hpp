//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Statistics / Counter for statistics
//

#pragma once

namespace freewebrtc::stat {

class Counter {
public:
    using ValueType = unsigned;
    void inc() noexcept;

    ValueType count() const noexcept;
private:
    ValueType m_value = 0;
};

//
// inlines
//
inline void Counter::inc() noexcept {
    m_value++;
}

inline Counter::ValueType Counter::count() const noexcept {
    return m_value;
}


}
