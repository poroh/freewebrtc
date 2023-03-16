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
    void inc() noexcept;
private:
    unsigned m_value = 0;
};

//
// inlines
//
inline void Counter::inc() noexcept {
    m_value++;
}


}
