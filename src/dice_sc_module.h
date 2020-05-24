#pragma once
#include <vector>
#include "dice_module.h"


namespace cq::event {
        struct MessageEvent;
}

namespace dice {
    class sc_module : public dice_module {
    public:
        uint roll_res=0;
        int current_san = 0;
        bool match(const cq::event::MessageEvent& e, const std::wstring& ws) override;
        void process(const cq::event::MessageEvent& e, const std::wstring& ws) override;
        bool check(const cq::event::MessageEvent& e);
    };
}
