#include "dice_sc_module.h"

#include <regex>
#include <string>

#include "cqsdk/cqsdk.h"
#include "dice_calculator.h"
#include "dice_exception.h"
#include "dice_msg_queue.h"
#include "dice_utils.h"

namespace cq::event {
    struct MessageEvent;
}

namespace dice {
    bool sc_module::match(const cq::event::MessageEvent &e, const std::wstring &ws) {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*sc[^]*", std::regex_constants::ECMAScript | std::regex_constants::icase);
        return std::regex_match(ws, re);
    }

    int sc_module::roll_exp2int(const std::wstring &ws) {
        std::wregex re(L"[^]*?=([0-9]+)");
        std::wsmatch m;
        std::regex_match(ws, m, re);
        if (m[1].first == m[1].second) {
            return 0;
        }
        return std::stoi(m[1]);
    }
    bool sc_module::check(const cq::event::MessageEvent &e) {
        // get san
        std::string san_str = std::string("san");
        current_san = utils::get_single_card_properties(e.target, utils::get_chosen_card(e.target), san_str);
        //    if (current_san<0 || current_san>100){
        //    throw exception::exception(msg::GetGlobalMsg("strPropertyInvalidError"));
        //    }
        // check
        std::uniform_int_distribution<int> gen100(1, 100);
        std::mt19937 ran(clock());
        int res = gen100(ran);
        check_val = res;
        if (res > current_san) {
            return false;
        } else {
            return true;
        }
    }
    void sc_module::process(const cq::event::MessageEvent &e, const std::wstring &ws) {
        std::wregex re(
            L"[\\s]*[\\.。．][\\s]*sc[\\s]*([0-9dk+\\-*x×÷\\(\\)\\^\\.]+)[\\s]*\\/"
            L"[\\s]*([0-9dk+\\-*x×÷\\(\\)\\^\\.]+)[\\s]*",
            std::regex_constants::ECMAScript | std::regex_constants::icase);
        //第一组为成功表达式 第二组为失败表达式
        std::wsmatch m;
        std::wstring dice_res_str;
        int change_val = 0;
        if (std::regex_match(ws, m, re)) {
            bool_res = check(e); // 设置了current_san和roll_res两个值
            int default_dice = utils::get_defaultdice(e.target);
            std::string check_res;
            std::wstring dice_exp;
            if (bool_res) {
                check_res = "strSuccess";
                dice_exp = m[1];
            } else {
                check_res = "strFailure";
                dice_exp = m[2];
            }

            if (dice_exp.empty()) {
                dice_res_str = L"0";
                change_val = 0;
            } else if (dice_exp.find_first_not_of(L"0123456789") == std::wstring::npos) {
                dice_res_str = dice_exp; // 减少固定值
                change_val = std::stoi(dice_exp);
            } else {
                dice_res_str = dice_calculator(dice_exp, default_dice).form_string();
                change_val = roll_exp2int(dice_res_str);
            }

            int after_san = current_san - change_val >= 0 ? current_san - change_val : 0;
            // int after_san = current_san - change_val;
            std::string character_card_name = utils::get_chosen_card(e.target);
            std::map<std::string, int> mp_character_card = {{std::string("理智"), after_san}};
            utils::set_character_card(e.target, character_card_name, mp_character_card);
            std::string str_san_change = std::string("减少") + cq::utils::ws2s(dice_res_str);
            dice::msg_queue::MsgQueue.add(
                e.target,
                utils::format_string(msg::GetGlobalMsg("strSanityCheck"),
                                     std::map<std::string, std::string>{{"nick", utils::get_nickname(e.target)},
                                                                        {"check_val", std::to_string(check_val)},
                                                                        {"check_res", msg::GetGlobalMsg(check_res)},
                                                                        {"before_san", std::to_string(current_san)},
                                                                        {"str_san_change", str_san_change},
                                                                        {"after_san", std::to_string(after_san)}}));
            // std::wstring res;
            // int RollCount = 1;
            // if (m[2].first != m[2].second) {
            //     RollCount = std::stoi(m[3]);
            // }

            // if (RollCount <= 0 || RollCount > 10) {
            // }

            // std::wstring dice(m[4]);
            // std::wstring reason(m[5]);

            // if (dice.empty()) dice = L"d";
            // int default_dice = utils::get_defaultdice(e.target);

            // res = dice_calculator(dice, default_dice).form_string();

            // for (int i = 1; i != RollCount; i++) {
            //     res += '\n';
            //     res += dice_calculator(dice, default_dice).form_string();
            // }

            // if (m[1].first == m[1].second) {
            // } else {
            //     dice::msg_queue::MsgQueue.add(
            //         e.target,
            //         utils::format_string(msg::GetGlobalMsg("strHiddenDice"),
            //                              std::map<std::string, std::string>{{"nick",
            //                              utils::get_nickname(e.target)}}));

            //     dice::msg_queue::MsgQueue.add(cq::Target(*e.target.user_id),
            //                               utils::format_string(msg::GetGlobalMsg("strRollHiddenDice"),
            //                                                    std::map<std::string, std::string>{
            //                                                        {"origin", utils::get_originname(e.target)},
            //                                                        {"nick", utils::get_nickname(e.target)},
            //                                                        {"reason", cq::utils::ws2s(reason)},
            //                                                        {"dice_expression", cq::utils::ws2s(res)}
            //                                                    }));
        }

        else {
            throw exception::dice_expression_invalid_error();
        }
    }
} // namespace dice