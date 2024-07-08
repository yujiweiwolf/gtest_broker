#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <x/x.h>
#include "coral/coral.h"
#include "feeder/feeder.h"
#include "coral/wal_reader.h"

namespace co {
    using namespace std;

    class SharedMemoryReader {
    public:
        SharedMemoryReader();
        ~SharedMemoryReader();
        void Init(string dir, string code);
    private:
        void HandleContract(MemQContract* data);
        void HandleTick(MemQTick* data);
        void HandleOrder(MemQOrder* data);
        void HandleKnock(MemQKnock* data);
    private:
        x::MMapReader feeder_reader_;
        string need_code_;
        map<string, MemQContract> all_contract_;
        int contract_num = 0;
        int tick_num = 0;
        int order_num = 0;
        int knock_num = 0;
    };
}
