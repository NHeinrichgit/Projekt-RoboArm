import usb.core
import time

class USBTimeoutError(TimeoutError):
    pass

def findUSBdevice(idVendor = 0x2341, idProduct = 0x0043, timeout = 60):
    print('Searching for USB device...')
    timeout_time = time.time() + timeout
    
    while(time.time()<timeout_time):
        dev = usb.core.find(False, idVendor, idProduct)
        if dev is not None:
            print('Device found!')
            return
        time.sleep(0.5)
    raise USBTimeoutError()

def getEndpoint(dev):
    #set configuration
    dev.set_configuration()
    #get configuration (contains multiple interfaces)
    cfg = dev.get_active_configuration()
    #get interface (contains multiple alternate settings)
    intf = cfg[(0,0)]
    #find endpoint
    ep = usb.util.find_descriptor(intf,
    # match the first OUT endpoint
    custom_match = \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_OUT)
    
    return ep

def sendCoordinates(dev, x, y):

    return