#include "read_shm.h"

namespace co {
    SharedMemoryReader::SharedMemoryReader() {
    }

    SharedMemoryReader::~SharedMemoryReader() {
    }

    void SharedMemoryReader::Init(string dir, string code) {
        LOG_INFO << "dir: " << dir << ", code: " << code;
        need_code_ = code;
        feeder_reader_.Open(dir, "meta");
        feeder_reader_.Open(dir, "data");

        const void* data = nullptr;
        while (true) {
            int32_t type = feeder_reader_.Next(&data);
            // LOG_INFO << "type: " << type;
            if (type == kMemTypeQContract) {
                MemQContract *contract = (MemQContract *) data;
                HandleContract(contract);
            } else if (type == kMemTypeQTick) {
                MemQTick *tick = (MemQTick *) data;
                HandleTick(tick);
            } else if (type == kMemTypeQOrder) {
                MemQOrder *order = (MemQOrder *) data;
                HandleOrder(order);
            } else if (type == kMemTypeQKnock) {
                MemQKnock *knock = (MemQKnock *) data;
                HandleKnock(knock);
            } else if (type == 0) {
                LOG_INFO << "not parse tyep: " << type;
                break;
            }
        }
    }

    void SharedMemoryReader::HandleContract(MemQContract* contract) {
        if (need_code_ == contract->code) {
            LOG_INFO << "contract, code: " << contract->code
                     << ", timestamp: " << contract->timestamp
                     << ", market: " << (int) contract->market
                     << ", underlying_code: " << contract->underlying_code
                     << ", pre_close: " << contract->pre_close
                     << ", upper_limit: " << contract->upper_limit
                     << ", lower_limit: " << contract->lower_limit
                     << ", pre_settle: " << contract->pre_settle
                     << ", pre_open_interest: " << contract->pre_open_interest
                     << ", multiple: " << contract->multiple
                     << ", price_step: " << contract->price_step
                     << ", dtype: " << (int) contract->dtype
                     << ", cp_flag: " << (int) contract->cp_flag;
        }
        all_contract_.insert(std::make_pair(contract->code, *contract));
    }

    void SharedMemoryReader::HandleTick(MemQTick* tick) {
        string code = tick->code;
         if (need_code_ == code) {
            if (auto it = all_contract_.find(tick->code); it != all_contract_.end()) {
                std::cout << "QTick{"
                          << "src: " << (int)tick->src
                          << ", dtype: " << (int)it->second.dtype
                          << ", timestamp: " << tick->timestamp
                          << ", code: " << code
                          << ", name: " << it->second.name
                          << ", market: " << (int)it->second.market
                          << ", pre_close: " << it->second.pre_close
                          << ", upper_limit: " << it->second.upper_limit
                          << ", lower_limit: " << it->second.lower_limit
                          << ", bp: " << tick->bp[0] << ", " << tick->bp[1] << ", " << tick->bp[2] << ", " << tick->bp[3] << ", " << tick->bp[4]
                          << ", " << tick->bp[5] << ", " << tick->bp[6] << ", " << tick->bp[7] << ", " << tick->bp[8] << ", " << tick->bp[9]
                          << ", bv: " << tick->bv[0] << ", " << tick->bv[1] << ", " << tick->bv[2] << ", " << tick->bv[3] << ", " << tick->bv[4]
                          << ", " << tick->bv[5] << ", " << tick->bv[6] << ", " << tick->bv[7] << ", " << tick->bv[8] << ", " << tick->bv[9]
                          << ", ap: " << tick->ap[0] << ", " << tick->ap[1] << ", " << tick->ap[2] << ", " << tick->ap[3] << ", " << tick->ap[4]
                          << ", " << tick->ap[5] << ", " << tick->ap[6] << ", " << tick->ap[7] << ", " << tick->ap[8] << ", " << tick->ap[9]
                          << ", av: " << tick->av[0] << ", " << tick->av[1] << ", " << tick->av[2] << ", " << tick->av[3] << ", " << tick->av[4]
                          << ", " << tick->av[5] << ", " << tick->av[6] << ", " << tick->av[7] << ", " << tick->av[8] << ", " << tick->av[9]
                          << ", status: " << (int)tick->state
                          << ", new_price: " << tick->new_price
                          << ", new_volume: " << tick->new_volume
                          << ", new_amount: " <<tick->new_amount
                          << ", sum_volume: " << tick->sum_volume
                          << ", sum_amount: " << tick->sum_amount
                          << ", open: " << tick->open
                          << ", new_bid_volume: " << tick->new_bid_volume
                          << ", new_bid_amount: " << tick->new_bid_amount
                          << ", new_ask_volume: " << tick->new_ask_volume
                          << ", new_ask_amount: " << tick->new_ask_amount
                          << ", open_interest: " << tick->open_interest
                          << ", pre_settle: " << it->second.pre_settle
                          << ", pre_open_interest: " << it->second.pre_open_interest
                          << ", close: " << tick->close
                          << ", settle: " << tick->settle
                          << ", multiple: " << it->second.multiple
                          << ", price_step: " << it->second.price_step
                          << ", list_date: " << it->second.list_date
                          << ", expire_date: " << it->second.expire_date
                          << ", exercise_price: " << it->second.exercise_price
                          << ", cp_flag: " << (int)it->second.cp_flag
                          << ", underlying_code: " << it->second.underlying_code
                          << ", date: " << it->second.date
                          << "}" << std::endl;
            }
        }
    }

    void SharedMemoryReader::HandleOrder(MemQOrder* order) {
        string code = order->code;
        if (need_code_ == code) {
            std::cout << "QOrder{timestamp: " << order->timestamp
                      << ", code: " << code
                      << ", order_no: " << order->order_no
                      << ", bs_flag: " << (int)order->bs_flag
                      << ", order_type: " << (int)order->order_type
                      << ", order_price: " << order->order_price
                      << ", order_volume: " << order->order_volume
                      << ", stream_no: " << order->stream_no
                      << "}" << std::endl;
        }
    }

    void SharedMemoryReader::HandleKnock(MemQKnock* knock) {
        string code = knock->code;
        if (need_code_ == code) {
            std::cout << "QKnock{timestamp: " << knock->timestamp
                      << ", code: " << code
                      << ", match_no: " << knock->match_no
                      << ", bid_order_no: " << knock->bid_order_no
                      << ", ask_order_no: " << knock->ask_order_no
                      << ", match_price: " << knock->match_price
                      << ", match_volume: " << knock->match_volume
                      << ", stream_no: " << knock->stream_no
                      << "}" << std::endl;
        }
    }
}
