#include "read_broker.h"

namespace co {
    SharedMemoryReader::SharedMemoryReader() {
    }

    SharedMemoryReader::~SharedMemoryReader() {
    }

    void SharedMemoryReader::Init(string dir, string code) {
        LOG_INFO << "dir: " << dir;
        reader_.Open(dir, "broker_req");
        reader_.Open(dir, "broker_rep");

        const void* data = nullptr;
        const char fund_id[] = "S1";
        auto get_req = [&](int32_t type, const void* data)-> bool {
            if (type == kMemTypeTradeOrderReq) {
                MemTradeOrderMessage *msg = (MemTradeOrderMessage *)data;
                if (strcmp(msg->fund_id, fund_id) == 0) {
                    return true;
                } else {
                    return false;
                }
            } else if (type == kMemTypeTradeWithdrawReq) {
                MemTradeWithdrawMessage *msg = (MemTradeWithdrawMessage *)data;
                if (strcmp(msg->fund_id, fund_id) == 0) {
                    return true;
                } else {
                    return false;
                }
            }
            return false;
        };
        while (true) {
            // int32_t type = reader_.Next(&data);
            // int32_t type = reader_.ConsumeWhere(&data, get_req, false);
            int32_t type = reader_.Consume(&data);
            LOG_INFO << "type: " << type;
            if (type == kMemTypeTradeOrderReq) {
                MemTradeOrderMessage* msg = (MemTradeOrderMessage*) data;
                SendTradeOrder(msg);
            } else if (type == kMemTypeTradeWithdrawReq) {
                MemTradeWithdrawMessage* msg = (MemTradeWithdrawMessage*) data;

            } else if (type == kMemTypeQTick) {

            } else if (type == kMemTypeQOrder) {

            } else if (type == kMemTypeQKnock) {

            } else if (type == 0) {
                LOG_INFO << "finish";
                break;
            }
        }
    }

    void SharedMemoryReader::SendTradeOrder(MemTradeOrderMessage* req) {
        LOG_INFO << "send order, fund: " << req->fund_id << ", fund: " << req->timestamp << ", id: " << req->id;
        int items_size = req->items_size;
        MemTradeOrder* first = (MemTradeOrder*)((char*)req + sizeof(MemTradeOrderMessage));
        for (int i = 0; i < items_size; ++i) {
            MemTradeOrder *order = first + i;
            LOG_INFO << "  order, info, code: " << order->code << ", volume: " << order->volume << ", price: " << order->price;
        }
    }

}
