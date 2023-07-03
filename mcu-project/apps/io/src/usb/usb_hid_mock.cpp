#include "usb_hid_mock.h"

UsbHidMock::UsbHidMock()
{}

int UsbHidMock::Init(void* message_queue, void* work_queue)
{
    return 0;
}

void UsbHidMock::Send(uint8_t* buffer, uint8_t message_length)
{}