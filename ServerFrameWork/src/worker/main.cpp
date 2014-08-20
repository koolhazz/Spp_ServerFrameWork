#include "defaultworker.h"

using namespace spp::worker;

int main(int argc, char* argv[])
{
    CServerBase* worker = new CDefaultWorker;
    worker->run(argc, argv);
    return 0;
}

