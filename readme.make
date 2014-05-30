== How to make it ==

To make it, you need to:

* Install xcode from app store

* Install macports

After install the dmg from http://www.macports.org/install.php
You need to execute : 

 sudo port -v selfupdate
 
* Install libsdl from macports

 sudo port install libsdl

Finally make it:

 make

== How to generate the Info.plist ==

Eventually you can use the Mac tool "ioreg -l" to enumerate monitor's usb properties... I think the most important is bcdDevice.
For 22", that's 8192, for 19", that's 4096, for now, I setup it in Info.plist about 15"(pid=72d0) is the same as 19" 4096...

The below is an example that I captured by "ioreg -l" for 19"

    | |   |   +-o PCAP MultiTouch Controller@fa130000  <class IOUSBDevice, id 0x10000026a, registered, matched, active, busy 0 (498 ms), retain 9>
    | |   |   | | {
    | |   |   | |   "sessionID" = 879366581
    | |   |   | |   "iManufacturer" = 1
    | |   |   | |   "bNumConfigurations" = 1
    | |   |   | |   "idProduct" = 29380
    | |   |   | |   "bcdDevice" = 4096
    | |   |   | |   "Bus Power Available" = 250
    | |   |   | |   "bMaxPacketSize0" = 64
    | |   |   | |   "USB Product Name" = "PCAP MultiTouch Controller"
    | |   |   | |   "iProduct" = 2
    | |   |   | |   "iSerialNumber" = 0
    | |   |   | |   "USB Address" = 4
    | |   |   | |   "bDeviceClass" = 0
    | |   |   | |   "locationID" = 18446744073610133504
    | |   |   | |   "bDeviceSubClass" = 0
    | |   |   | |   "IOUserClientClass" = "IOUSBDeviceUserClientV2"
    | |   |   | |   "PortNum" = 3
    | |   |   | |   "IOCFPlugInTypes" = {"9dc7b780-9ec0-11d4-a54f-000a27052861"="IOUSBFamily.kext/Contents/PlugIns/IOUSBLib.bundle"}
    | |   |   | |   "bDeviceProtocol" = 0
    | |   |   | |   "USB Vendor Name" = "eGalax_eMPIA Technology Inc."
    | |   |   | |   "Device Speed" = 1
    | |   |   | |   "idVendor" = 3823
    | |   |   | |   "Requested Power" = 50
    | |   |   | |   "IOGeneralInterest" = "IOCommand is not serializable"
    | |   |   | |   "Low Power Displayed" = No
    | |   |   | | }

== How to let SimpleSimulator supports new monitor ==

After you prepare the right Info.plist, then you need to change SimpleSimulator.cpp to support the new monitor...
Find this section in the code, change / add them as you wish...

        switch (screen_size) {
        case 22:
                handle = hid_open(0x0eef, 0x7224, NULL);
                break;
        case 19:
                handle = hid_open(0x0eef, 0x72c4, NULL);
                break;
        case 15:
                handle = hid_open(0x0eef, 0x72d0, NULL);
                break;
        default:
                break;
        }

