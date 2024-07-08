#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include "read_shm.h"


using namespace std;
using namespace co;
namespace po = boost::program_options;

const string kVersion = "v1.0.1";

int main(int argc, char* argv[]) {
    try {
        string dir = argv[1];
        string need_code = argv[2];
        shared_ptr<SharedMemoryReader> db_writer = make_shared<SharedMemoryReader>();
        db_writer->Init(dir, need_code);
        while(true) {
            x::Sleep(1000);
        }
        LOG_INFO << "server is stopped.";
    } catch (std::exception& e) {
        LOG_INFO << "server is crashed, " << e.what();
        throw e;
    }
    return 0;
}
