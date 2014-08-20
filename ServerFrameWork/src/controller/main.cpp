#include "misc.h"
#include "defaultctrl.h"

using namespace comm::base;
using namespace spp::ctrl;

int main(int argc, char* argv[])
{
    CServerBase* ctrl = new CDefaultCtrl;
    ctrl->run(argc, argv);
    return 0;
}

