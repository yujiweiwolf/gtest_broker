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
#include "/home/work/sys/develop/libmembroker/src/mem_broker/mem_struct.h"

namespace co {
    using namespace std;

    class SharedMemoryReader {
    public:
        SharedMemoryReader();
        ~SharedMemoryReader();
        void Init(string dir, string code);
    private:
        void SendTradeOrder(MemTradeOrderMessage* req);
    private:
        x::MMapReader reader_;
    };
}
