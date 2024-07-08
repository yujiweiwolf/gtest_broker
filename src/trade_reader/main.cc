#include <iostream>
#include <boost/program_options.hpp>
#include <x/x.h>
#include "coral/wal_reader.h"
#include "feeder/feeder.h"
#include <boost/lockfree/queue.hpp>
#include "gtest/gtest.h"


using namespace std;
using namespace co;
namespace po = boost::program_options;

// const string file = "/home/work/sys/develop/feeder_parse_czce_history/data/feeder_20200813_20220630104445171.wal";
// https://www.sse.com.cn/assortment/fund/etf/price/ 上交所ETF文件

int main(int argc, char* argv[]) {
    EXPECT_EQ(2, 2) << ", a = 1";
    string file = argv[1];
    // string need_code = argv[2];
    // string file = "/home/work/sys/trade_20220411_20220411085255001.wal";
    // 记录每次开始的值
    co::fbs::TradeAssetT asset_;
    bool query_pos_flag = false;
    std::map<string, shared_ptr<co::fbs::TradePositionT>> all_pos_;
    std::map<string, std::vector<co::fbs::TradeKnockT>> all_knock_;  // 申赎的成交回报, key order_no
    string etf_code = "510050.SH";
    int etf_unit = 900000;
    std::map<string, int> etf_composite_;  // etf成分股的数量
    // 解析ETF文件
    {
        string dataPath = "../conf/51005007032.ETF";
        std::fstream infile;
        infile.open(dataPath, std::ios::in);   // 打开文件
        if (!infile.is_open()) {
            LOG_ERROR << "open file " << dataPath << " failed! ";
            return 0;
        }
        std::vector<std::string> data;
        std::string s;
        while (std::getline(infile, s)) {     // 按行读取
            data.push_back(s);
        }
        std::vector<string> vec_tmp;
        for (auto& it : data) {
            vec_tmp.clear();
            auto pos = it.find("|");
            if (pos != it.npos) {
                string &line = it;
                x::Split(&vec_tmp, line, "|");
                if (vec_tmp.size() > 3) {
                    string code = x::Trim(vec_tmp[0]);
                    string name = x::Trim(vec_tmp[1]);
                    int num = atoi(x::Trim(vec_tmp[2]).c_str());
                    // int market = atoi(x::Trim(vec_tmp[3]).c_str());
                    string std_code =  code + ".SH";
                    // LOG_INFO << std_code << ", name: " << name << ", num: " << num;
                    etf_composite_.insert(std::make_pair(std_code, num));
                }
            }
        }
    }
    EXPECT_GT(0.001, 0);
    try {
        co::WALReader reader;
        reader.Open(file.c_str());
        while (true) {
            std::string raw;
            int64_t type = reader.Read(&raw);
            // LOG_INFO << "type: " << type;
            if (raw.empty()) {
                break;
            }
            switch (type) {
                case kFuncFBTradeAsset: {
                    auto q = flatbuffers::GetRoot<co::fbs::TradeAsset>(raw.data());
                    std::string fund_id = q->fund_id() ? q->fund_id()->str() : "";
                    int64_t timestamp = q->timestamp();
                    int64_t balance = q->balance();
                    int64_t usable = q->usable();
                    EXPECT_GE(balance, 0);
                    EXPECT_GE(usable, 0);
                    if (usable > 0) {
                        EXPECT_GT(balance, 0);
                    }
                    LOG_INFO << "fund_id: " << fund_id << ", timestamp: " << timestamp << ", balance: " << balance
                             << ", usable: " << usable;
                    if (asset_.fund_id.empty()) {
                        asset_.fund_id = fund_id;
                        asset_.balance = balance;
                        asset_.usable = usable;
                    } else {
                        // 与计算的值比较
                        EXPECT_NEAR(usable, asset_.usable, 100);
                    }
                    break;
                }
                case kFuncFBTradePosition: {
                    auto cur = flatbuffers::GetRoot<co::fbs::TradePosition>(raw.data());
                    std::string fund_id = cur->fund_id() ? cur->fund_id()->str() : "";
                    int64_t timestamp = cur->timestamp();
                    std::string code = cur->code() ? cur->code()->str() : "";
//                    LOG_INFO << code << ", timestamp: " << timestamp
//                             << ", long_volume: " << cur->long_volume()
//                             << ", long_can_close: " << cur->long_can_close()
//                             << ", long_market_value: " << cur->long_market_value()
//                             << ", short_volume: " << cur->short_volume()
//                             << ", short_can_close: " << cur->short_can_close()
//                             << ", short_market_value: " << cur->short_market_value();
                    EXPECT_GE(cur->long_volume(), 0);
                    EXPECT_GE(cur->long_can_close(), 0);
                    if (cur->long_can_close() > 0) {
                        EXPECT_GT(cur->long_volume(), 0);
                    }
//                    if (cur->long_volume() > 0) {
//                        EXPECT_GT(cur->long_market_value(), 0);
//                    }
                    if (!query_pos_flag) {
                        shared_ptr<co::fbs::TradePositionT> pos = std::make_shared<co::fbs::TradePositionT>();
                        pos->code = code;
                        pos->long_volume = cur->long_volume();
                        pos->long_can_close = cur->long_can_close();
                        pos->long_market_value = cur->long_market_value();
                        pos->short_volume = cur->short_volume();
                        pos->short_can_close = cur->short_can_close();
                        all_pos_.insert(std::make_pair(code, pos));
                    } else {
                        if (auto it = all_pos_.find(code); it != all_pos_.end()) {
                            shared_ptr<co::fbs::TradePositionT> pos = it->second;
                            EXPECT_EQ(pos->long_volume, cur->long_volume());
                            EXPECT_EQ(pos->long_can_close, cur->long_can_close());
                            EXPECT_EQ(pos->short_can_close, cur->short_can_close());
                        }
                    }
                    break;
                }
                case kFuncFBTradeKnock: {
                    query_pos_flag = true;  // 成交回报前的持仓，是初始化数据
                    auto cur = flatbuffers::GetRoot<co::fbs::TradeKnock>(raw.data());
                    LOG_INFO << ToString(*cur);
                    std::string fund_id = cur->fund_id() ? cur->fund_id()->str() : "";
                    std::string inner_match_no = cur->inner_match_no() ? cur->inner_match_no()->str() : "";
                    std::string match_no = cur->match_no() ? cur->match_no()->str() : "";
                    std::string order_no = cur->order_no() ? cur->order_no()->str() : "";
                    std::string code = cur->code() ? cur->code()->str() : "";
                    std::string batch_no = cur->batch_no() ? cur->batch_no()->str() : "";
                    std::string error = cur->error() ? cur->error()->str() : "";
                    EXPECT_TRUE(fund_id.length() > 0);
                    EXPECT_TRUE(code.length() > 0);
                    EXPECT_TRUE(order_no.length() > 0);
                    EXPECT_TRUE(match_no.length() > 0);
                    EXPECT_TRUE(inner_match_no.length() > 0);
                    int64_t market = cur->market();
                    EXPECT_GT(market, 0);
                    int64_t bs_flag = cur->bs_flag();
                    EXPECT_GT(bs_flag, 0);
                    int64_t oc_flag = cur->oc_flag();
                    EXPECT_GE(bs_flag, 0);
                    int64_t match_type = cur->match_type();
                    EXPECT_GT(match_type, 0);
                    int64_t match_volume = cur->match_volume();
                    if (bs_flag != co::kBsFlagCreate && bs_flag != co::kBsFlagRedeem) {
                        EXPECT_GT(match_volume, 0);
                    }
                    double match_price = cur->match_price();
                    double match_amount = cur->match_amount();
                    if (match_type == co::kMatchTypeOK) {
                        if (bs_flag == co::kBsFlagBuy || bs_flag == co::kBsFlagSell) {
                            EXPECT_GT(match_price, 0);
                            EXPECT_GT(match_amount, 0);
                        }
                    } else if (match_type == co::kMatchTypeWithdrawOK) {
                        EXPECT_DOUBLE_EQ(match_price, 0);
                        EXPECT_DOUBLE_EQ(match_amount, 0);
                    } else if (match_type == co::kMatchTypeFailed) {
                        EXPECT_DOUBLE_EQ(match_price, 0);
                        EXPECT_DOUBLE_EQ(match_amount, 0);
                        EXPECT_GT(error.length(), 0);
                    }
                    if (match_type == co::kMatchTypeOK) {
                        shared_ptr<co::fbs::TradePositionT> pos;
                        auto it = all_pos_.find(code);
                        if (it != all_pos_.end()) {
                            pos = it->second;
                        } else {
                            pos = std::make_shared<co::fbs::TradePositionT>();
                            all_pos_.insert(std::make_pair(code, pos));
                        }
                        if (bs_flag == co::kBsFlagBuy) {
                            asset_.usable -= match_amount;
                            pos->long_volume += match_volume;
                        } else if (bs_flag == co::kBsFlagSell) {
                            if (oc_flag == co::kOcFlagAuto) {
                                asset_.usable += match_amount;
                                pos->long_volume -= match_volume;
                                pos->long_can_close -= match_volume;
                            } else if (oc_flag == co::kOcFlagClose) {
                                pos->short_can_close -= match_volume;
                            }
                        } else if (bs_flag == co::kBsFlagCreate || bs_flag == co::kBsFlagRedeem) {
                            co::fbs::TradeKnockT knock;
                            knock.code = code;
                            knock.bs_flag = bs_flag;
                            knock.order_no = order_no;
                            knock.match_volume = match_volume;
                            knock.match_price = match_price;
                            knock.match_amount = match_amount;
                            auto it = all_knock_.find(order_no);
                            if (it != all_knock_.end()) {
                                it->second.push_back(knock);
                                // 数量可修改
                                if (it->second.size() == (etf_composite_.size() + 1)) {
                                    // 检查成分股、增减券、现金代替换
                                    bool cash_flag_ = false;
                                    bool etf_flag_ = false;
                                    for (auto &itor: it->second) {
                                        if (itor.code == etf_code) {
                                            if (itor.match_volume > 0 && itor.match_amount) {
                                                EXPECT_TRUE(false) << "申赎时， match_volume与match_amount不能同时有值 ";
                                            }
                                            if (itor.match_volume == etf_unit && itor.match_amount < 0.001) {
                                                etf_flag_ = true;
                                            }
                                            if (itor.match_volume == 0 && itor.match_amount > 0) {
                                                cash_flag_ = true;
                                            }
                                        } else {
                                            EXPECT_DOUBLE_EQ(itor.match_price, 0);
                                            EXPECT_DOUBLE_EQ(itor.match_amount, 0);
                                            if (auto iter = etf_composite_.find(code); iter != etf_composite_.end()) {

                                            }
                                        }
                                    }
                                    if (!cash_flag_) {
                                        EXPECT_TRUE(false) << "没有ETF现金替换的成交回报";
                                    }
                                    if (!etf_flag_) {
                                        EXPECT_TRUE(false) << "没有ETF增减券的成交回报";
                                    }
                                }
                            } else {
                                std::vector<co::fbs::TradeKnockT> tmp_vec;
                                tmp_vec.push_back(knock);
                                all_knock_.insert(std::make_pair(order_no, tmp_vec));
                            }
                            if (bs_flag == co::kBsFlagCreate) {
                                if (code == etf_code) {
                                    pos->long_volume -= match_volume;
                                    pos->long_can_close -= match_volume;
                                } else {
                                    pos->long_volume += match_volume;
                                    pos->long_can_close += match_volume;
                                }
                            } else {
                                if (code == etf_code) {
                                    pos->long_volume += match_volume;
                                    pos->long_can_close += match_volume;
                                } else {
                                    pos->long_volume -= match_volume;
                                    pos->long_can_close -= match_volume;
                                }
                            }
                        }
                    }
                    break;
                }
                case kFuncFBTradePolicy: {
                    auto rep = flatbuffers::GetRoot<co::fbs::TradeOrderMessage>(reinterpret_cast<void *>(raw.data()));
                    LOG_INFO << ToString(*rep);
                    std::string fund_id = rep->fund_id() ? rep->fund_id()->str() : "";
                    std::string error = rep->error() ? rep->error()->str() : "";
                    std::string batch_no = rep->batch_no() ? rep->batch_no()->str() : "";
                    std::string message_id = rep->id() ? rep->id()->str() : "";
                    int size = rep->items()->size();
                    if (size > 1 && batch_no.empty()) {
                        EXPECT_TRUE(false) << message_id << ", batch order, batch_no is empty;";
                    }
                    for (const auto& item : *rep->items()) {
                        std::string code = item->code() ? item->code()->str() : "";
                        std::string order_no = item->order_no() ? item->order_no()->str() : "";
                        std::string error = item->error() ? item->error()->str() : "";
                        if (order_no.empty() && error.empty()) {
                            EXPECT_TRUE(false) << "order code: " << code << ", order_no and error both are empty;";
                        }
                    }
                    break;
                }
                default: {
                    LOG_INFO << "type: " << type;
                    break;
                }
            }
        }
    } catch (std::exception& e) {
        LOG_INFO << "server is crashed, " << e.what();
        return 2;
    } catch (...) {
        LOG_INFO << "server is crashed, unknown reason";
        return 3;
    }
    return 0;
}

