/* Teensy Loader, JAVA JNI layer
 * Program and Reboot Teensy Board with HalfKay Bootloader
 * http://www.pjrc.com/teensy/loader_cli.html
 * Copyright 2008-2010, PJRC.COM, LLC
 *
 * You may redistribute this program and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

/* Want to incorporate this code into a proprietary application??
 * Just email paul@pjrc.com to ask.  Usually it's not a problem,
 * but you do need to ask to use this code in any way other than
 * those permitted by the GNU General Public License, version 3  */

/* For non-root permissions on ubuntu or similar udev-based linux
 * http://www.pjrc.com/teensy/49-teensy.rules
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "jni.h"

// USB Access Functions
static int teensy_open(void);
static int teensy_write(void *buf, int len, double timeout);
static void teensy_close(void);
static int hard_reboot(void);

// JNI stubs
JNIEXPORT jint JNICALL Java_org_waxbee_Teensy_version(JNIEnv *env, jobject obj)
{
	jint ret = 10;
	
	return ret;
}

JNIEXPORT jboolean JNICALL Java_org_waxbee_Teensy_open(JNIEnv *env, jobject obj)
{
	int ret = teensy_open();
	
	return ret?JNI_TRUE:JNI_FALSE;
}

JNIEXPORT void JNICALL Java_org_waxbee_Teensy_close(JNIEnv *env, jobject obj)
{
	teensy_close();
}

JNIEXPORT jboolean JNICALL Java_org_waxbee_Teensy_write(JNIEnv *env, jobject obj, jbyteArray bytes, jint len, jdouble timeout)
{
	jbyte *buf = (*env)->GetByteArrayElements(env, bytes, 0);

	int ret = teensy_write(buf,len,timeout);
	
	(*env)->ReleaseByteArrayElements(env, bytes, buf, 0);
	
	return ret?JNI_TRUE:JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_org_waxbee_Teensy_hard_reboot(JNIEnv *env, jobject obj)
{
	int ret = hard_reboot();
	
	return ret?JNI_TRUE:JNI_FALSE;
}

/****************************************************************/
/*                                                              */
/*             USB Access - libusb (Linux & FreeBSD)            */
/*                                                              */
/****************************************************************/

#if defined(USE_LIBUSB)

// http://libusb.sourceforge.net/doc/index.html
#include <usb.h>

static usb_dev_handle * open_usb_device(int vid, int pid)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *h;
	char buf[128];
	int r;

	usb_init();
	usb_find_busses();
	usb_find_devices();
	//printf_verbose("\nSearching for USB device:\n");
	for (bus = usb_get_busses(); bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			//printf_verbose("bus \"%s\", device \"%s\" vid=%04X, pid=%04X\n",
			//	bus->dirname, dev->filename,
			//	dev->descriptor.idVendor,
			//	dev->descriptor.idProduct
			//);
			if (dev->descriptor.idVendor != vid) continue;
			if (dev->descriptor.idProduct != pid) continue;
			h = usb_open(dev);
			if (!h) {
				printf_verbose("Found device but unable to open");
				continue;
			}
			#ifdef LIBUSB_HAS_GET_DRIVER_NP
			r = usb_get_driver_np(h, 0, buf, sizeof(buf));
			if (r >= 0) {
				r = usb_detach_kernel_driver_np(h, 0);
				if (r < 0) {
					usb_close(h);
					printf_verbose("Device is in use by \"%s\" driver", buf);
					continue;
				}
			}
			#endif
			// Mac OS-X - removing this call to usb_claim_interface() might allow
			// this to work, even though it is a clear misuse of the libusb API.
			// normally Apple's IOKit should be used on Mac OS-X
			r = usb_claim_interface(h, 0);
			if (r < 0) {
				usb_close(h);
				printf_verbose("Unable to claim interface, check USB permissions");
				continue;
			}
			return h;
		}
	}
	return NULL;
}

static usb_dev_handle *libusb_teensy_handle = NULL;

static int teensy_open(void)
{
	teensy_close();
	libusb_teensy_handle = open_usb_device(0x16C0, 0x0478);
	if (libusb_teensy_handle) return 1;
	return 0;
}

static int teensy_write(void *buf, int len, double timeout)
{
	int r;

	if (!libusb_teensy_handle) return 0;
	r = usb_control_msg(libusb_teensy_handle, 0x21, 9, 0x0200, 0, (char *)buf,
		len, (int)(timeout * 1000.0));
	if (r < 0) return 0;
	return 1;
}

static void teensy_close(void)
{
	if (!libusb_teensy_handle) return;
	usb_release_interface(libusb_teensy_handle, 0);
	usb_close(libusb_teensy_handle);
	libusb_teensy_handle = NULL;
}

static int hard_reboot(void)
{
	usb_dev_handle *rebootor;
	int r;

	rebootor = open_usb_device(0x16C0, 0x0477);
	if (!rebootor) return 0;
	r = usb_control_msg(rebootor, 0x21, 9, 0x0200, 0, "reboot", 6, 100);
	usb_release_interface(rebootor, 0);
	usb_close(rebootor);
	if (r < 0) return 0;
	return 1;
}

#endif


/****************************************************************/
/*                                                              */
/*               USB Access - Microsoft WIN32                   */
/*                                                              */
/****************************************************************/

#if defined(USE_WIN32)

// http://msdn.microsoft.com/en-us/library/ms790932.aspx
#include <windows.h>
#include <setupapi.h>

#ifdef __MINGW64__
 // tweaks done in header files for mingw64
//#include "win64/hidpi.h"
#include "win64/hidclass.h"
#include "win64/hidsdi.h"
#else
 // mingw32
#include <ddk/hidclass.h>
#include <ddk/hidsdi.h>
#endif

static HANDLE open_usb_device(int vid, int pid)
{
	GUID guid;
	HDEVINFO info;
	DWORD index, required_size;
	SP_DEVICE_INTERFACE_DATA iface;
	SP_DEVICE_INTERFACE_DETAIL_DATA *details;
	HIDD_ATTRIBUTES attrib;
	HANDLE h;
	BOOL ret;

	HidD_GetHidGuid(&guid);
	info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (info == INVALID_HANDLE_VALUE) return NULL;
	for (index=0; 1 ;index++) {
		iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		ret = SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface);
		if (!ret) {
			SetupDiDestroyDeviceInfoList(info);
			break;
		}
		SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &required_size, NULL);
		details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(required_size);
		if (details == NULL) continue;
		memset(details, 0, required_size);
		details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		ret = SetupDiGetDeviceInterfaceDetail(info, &iface, details,
			required_size, NULL, NULL);
		if (!ret) {
			free(details);
			continue;
		}
		h = CreateFile(details->DevicePath, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED, NULL);
		free(details);
		if (h == INVALID_HANDLE_VALUE) continue;
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		ret = HidD_GetAttributes(h, &attrib);
		if (!ret) {
			CloseHandle(h);
			continue;
		}
		if (attrib.VendorID != vid || attrib.ProductID != pid) {
			CloseHandle(h);
			continue;
		}
		SetupDiDestroyDeviceInfoList(info);
		return h;
	}
	return NULL;
}

static int write_usb_device(HANDLE h, void *buf, int len, int timeout)
{
	static HANDLE event = NULL;
	unsigned char tmpbuf[1040];
	OVERLAPPED ov;
	DWORD n, r;

	if (len > sizeof(tmpbuf) - 1) return 0;
	if (event == NULL) {
		event = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (!event) return 0;
	}
	ResetEvent(&event);
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = event;
	tmpbuf[0] = 0;
	memcpy(tmpbuf + 1, buf, len);
	if (!WriteFile(h, tmpbuf, len + 1, NULL, &ov)) {
		if (GetLastError() != ERROR_IO_PENDING) return 0;
		r = WaitForSingleObject(event, timeout);
		if (r == WAIT_TIMEOUT) {
			CancelIo(h);
			return 0;
		}
		if (r != WAIT_OBJECT_0) return 0;
	}
	if (!GetOverlappedResult(h, &ov, &n, FALSE)) return 0;
	if (n <= 0) return 0;
	return 1;
}

/*void print_win32_err(void)
{
        char buf[256];
        DWORD err;

        err = GetLastError();
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                0, buf, sizeof(buf), NULL);
        printf("err %ld: %s\n", err, buf);
}
*/
static HANDLE win32_teensy_handle = NULL;

static int teensy_open(void)
{
	teensy_close();
	win32_teensy_handle = open_usb_device(0x16C0, 0x0478);
	if (win32_teensy_handle) return 1;
	return 0;
}

static int teensy_write(void *buf, int len, double timeout)
{
	int r;
	if (!win32_teensy_handle) return 0;
	r = write_usb_device(win32_teensy_handle, buf, len, (int)(timeout * 1000.0));
	//if (!r) print_win32_err();
	return r;
}

static void teensy_close(void)
{
	if (!win32_teensy_handle) return;
	CloseHandle(win32_teensy_handle);
	win32_teensy_handle = NULL;
}

static int hard_reboot(void)
{
	HANDLE rebootor;
	int r;

	rebootor = open_usb_device(0x16C0, 0x0477);
	if (!rebootor) return 0;
	r = write_usb_device(rebootor, "reboot", 6, 100);
	CloseHandle(rebootor);
	return r;
}

#endif



/****************************************************************/
/*                                                              */
/*             USB Access - Apple's IOKit, Mac OS-X             */
/*                                                              */
/****************************************************************/

#if defined(USE_APPLE_IOKIT)

// http://developer.apple.com/technotes/tn2007/tn2187.html
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDDevice.h>

struct usb_list_struct {
	IOHIDDeviceRef ref;
	int pid;
	int vid;
	struct usb_list_struct *next;
};

static struct usb_list_struct *usb_list=NULL;
static IOHIDManagerRef hid_manager=NULL;

static void attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
	CFTypeRef type;
	struct usb_list_struct *n, *p;
	int32_t pid, vid;

	if (!dev) return;
	type = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDVendorIDKey));
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID()) return;
	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, &vid)) return;
	type = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDProductIDKey));
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID()) return;
	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, &pid)) return;
	n = (struct usb_list_struct *)malloc(sizeof(struct usb_list_struct));
	if (!n) return;
	//printf("attach callback: vid=%04X, pid=%04X\n", vid, pid);
	n->ref = dev;
	n->vid = vid;
	n->pid = pid;
	n->next = NULL;
	if (usb_list == NULL) {
		usb_list = n;
	} else {
		for (p = usb_list; p->next; p = p->next) ;
		p->next = n;
	}
}

static void detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
	struct usb_list_struct *p, *tmp, *prev=NULL;

	p = usb_list;
	while (p) {
		if (p->ref == dev) {
			if (prev) {
				prev->next = p->next;
			} else {
				usb_list = p->next;
			}
			tmp = p;
			p = p->next;
			free(tmp);
		} else {
			prev = p;
			p = p->next;
		}
	}
}

static void init_hid_manager(void)
{
	CFMutableDictionaryRef dict;
	IOReturn ret;

	if (hid_manager) return;
	hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
		if (hid_manager) CFRelease(hid_manager);
		printf_verbose("no HID Manager - maybe this is a pre-Leopard (10.5) system?\n");
		return;
	}
	dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (!dict) return;
	IOHIDManagerSetDeviceMatching(hid_manager, dict);
	CFRelease(dict);
	IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, attach_callback, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, detach_callback, NULL);
	ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
	if (ret != kIOReturnSuccess) {
		IOHIDManagerUnscheduleFromRunLoop(hid_manager,
			CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		CFRelease(hid_manager);
		printf_verbose("Error opening HID Manager");
	}
}

static void do_run_loop(void)
{
	while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) ;
}

static IOHIDDeviceRef open_usb_device(int vid, int pid)
{
	struct usb_list_struct *p;
	IOReturn ret;

	init_hid_manager();
	do_run_loop();
	for (p = usb_list; p; p = p->next) {
		if (p->vid == vid && p->pid == pid) {
			ret = IOHIDDeviceOpen(p->ref, kIOHIDOptionsTypeNone);
			if (ret == kIOReturnSuccess) return p->ref;
		}
	}
	return NULL;
}

static void close_usb_device(IOHIDDeviceRef dev)
{
	struct usb_list_struct *p;

	do_run_loop();
	for (p = usb_list; p; p = p->next) {
		if (p->ref == dev) {
			IOHIDDeviceClose(dev, kIOHIDOptionsTypeNone);
			return;
		}
	}
}

static IOHIDDeviceRef iokit_teensy_reference = NULL;

static int teensy_open(void)
{
	teensy_close();
	iokit_teensy_reference = open_usb_device(0x16C0, 0x0478);
	if (iokit_teensy_reference) return 1;
	return 0;
}

static int teensy_write(void *buf, int len, double timeout)
{
	IOReturn ret;

	// timeouts do not work on OS-X
	// IOHIDDeviceSetReportWithCallback is not implemented
	// even though Apple documents it with a code example!
	// submitted to Apple on 22-sep-2009, problem ID 7245050
	if (!iokit_teensy_reference) return 0;
	ret = IOHIDDeviceSetReport(iokit_teensy_reference,
		kIOHIDReportTypeOutput, 0, buf, len);
	if (ret == kIOReturnSuccess) return 1;
	return 0;
}

static void teensy_close(void)
{
	if (!iokit_teensy_reference) return;
	close_usb_device(iokit_teensy_reference);
	iokit_teensy_reference = NULL;
}

static int hard_reboot(void)
{
	IOHIDDeviceRef rebootor;
	IOReturn ret;

	rebootor = open_usb_device(0x16C0, 0x0477);
	if (!rebootor) return 0;
	ret = IOHIDDeviceSetReport(rebootor,
		kIOHIDReportTypeOutput, 0, (uint8_t *)("reboot"), 6);
	close_usb_device(rebootor);
	if (ret == kIOReturnSuccess) return 1;
	return 0;
}

#endif



/****************************************************************/
/*                                                              */
/*              USB Access - BSD's UHID driver                  */
/*                                                              */
/****************************************************************/

#if defined(USE_UHID)

// Thanks to Todd T Fries for help getting this working on OpenBSD
// and to Chris Kuethe for the initial patch to use UHID.

#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <dev/usb/usb.h>
#ifndef USB_GET_DEVICEINFO
#include <dev/usb/usb_ioctl.h>
#endif

static int open_usb_device(int vid, int pid)
{
	int r, fd;
	DIR *dir;
	struct dirent *d;
	struct usb_device_info info;
	char buf[256];

	dir = opendir("/dev");
	if (!dir) return -1;
	while ((d = readdir(dir)) != NULL) {
		if (strncmp(d->d_name, "uhid", 4) != 0) continue;
		snprintf(buf, sizeof(buf), "/dev/%s", d->d_name);
		fd = open(buf, O_RDWR);
		if (fd < 0) continue;
		r = ioctl(fd, USB_GET_DEVICEINFO, &info);
		if (r < 0) {
			// NetBSD: added in 2004
			// OpenBSD: added November 23, 2009
			// FreeBSD: missing (FreeBSD 8.0) - USE_LIBUSB works!
			die("Error: your uhid driver does not support"
			  " USB_GET_DEVICEINFO, please upgrade!\n");
			close(fd);
			closedir(dir);
			exit(1);
		}
		//printf("%s: v=%d, p=%d\n", buf, info.udi_vendorNo, info.udi_productNo);
		if (info.udi_vendorNo == vid && info.udi_productNo == pid) {
			closedir(dir);
			return fd;
		}
		close(fd);
	}
	closedir(dir);
	return -1;
}

static int uhid_teensy_fd = -1;

static int teensy_open(void)
{
	teensy_close();
	uhid_teensy_fd = open_usb_device(0x16C0, 0x0478);
	if (uhid_teensy_fd < 0) return 0;
	return 1;
}

static int teensy_write(void *buf, int len, double timeout)
{
	int r;

	// TODO: implement timeout... how??
	r = write(uhid_teensy_fd, buf, len);
	if (r == len) return 1;
	return 0;
}

static void teensy_close(void)
{
	if (uhid_teensy_fd >= 0) 
	{
		close(uhid_teensy_fd);
		uhid_teensy_fd = -1;
	}
}

static int hard_reboot(void)
{
	int r, rebootor_fd;

	rebootor_fd = open_usb_device(0x16C0, 0x0477);
	if (rebootor_fd < 0) return 0;
	r = write(rebootor_fd, "reboot", 6);
	delay(0.1);
	close(rebootor_fd);
	if (r == 6) return 1;
	return 0;
}

#endif

static void delay(double seconds)
{
	#ifdef WIN32
	Sleep(seconds * 1000.0);
	#else
	usleep(seconds * 1000000.0);
	#endif
}

#if defined(WIN32)
#define strcasecmp stricmp
#endif
