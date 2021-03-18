#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x59253af, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x83be3e36, __VMLINUX_SYMBOL_STR(usbnet_disconnect) },
	{ 0x5c38bb6, __VMLINUX_SYMBOL_STR(usbnet_probe) },
	{ 0xccbad838, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0x955f9999, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0x8a363b8e, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0x53338a71, __VMLINUX_SYMBOL_STR(__dev_kfree_skb_any) },
	{ 0xea421b3b, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xbe004f54, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0x1eb60ed2, __VMLINUX_SYMBOL_STR(usbnet_suspend) },
	{ 0x9b77584b, __VMLINUX_SYMBOL_STR(usbnet_resume) },
	{ 0x3ecd7d27, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x70d2da1f, __VMLINUX_SYMBOL_STR(usb_control_msg) },
	{ 0xecf0f2f0, __VMLINUX_SYMBOL_STR(usbnet_get_endpoints) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v1E0Ep9025d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v1E0Ep9001d*dc*dsc*dp*ic*isc*ip*in*");
