#include "usb_hid_mock.h"

UsbHidMock::UsbHidMock(Logger& logger) : logger_{logger}
{}

int UsbHidMock::Init(void* message_queue, void* work_queue)
{}

void UsbHidMock::Send(uint8_t* buffer, uint8_t message_length)
{}