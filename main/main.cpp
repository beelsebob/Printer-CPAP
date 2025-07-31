#include "PrinterCPAP.hpp"

extern "C" void app_main(void)
{
    pcp::PrinterCPAP cpap;
    cpap.run();
}
