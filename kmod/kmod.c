#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "linux/mm.h"
#include <linux/debugfs.h>
#include <linux/mm_types.h>
#include <linux/pagemap.h>
#include <asm/page.h>
#include <linux/pid.h>
#include <asm/pgtable.h>
#include <linux/fd.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/pci.h>

#define BUFFER_SIZE 1024
static DEFINE_MUTEX(kmod_mutex);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("OS LAB2");
MODULE_VERSION("1.0");

static struct dentry *kmod_root;
static struct dentry *kmod_args_file;
static struct dentry *kmod_result_file;

static struct dentry *struct_dentry;
static struct pci_dev *struct_pci_dev;


static int kmod_result_open(struct seq_file *sf, void *data);
struct dentry *kmod_get_dentry(void);
struct pci_dev *kmod_get_pci_device(struct pci_dev *from);
void set_result(void);

static int kmod_read(
        struct seq_file *sf,
        void *data
) {
    set_result();

	seq_printf(sf, "=============DENTRY=============\n");
    if (struct_dentry) {
        seq_printf(sf, "Dentry {d_name: %s, d_parent: %s, inode: %lu}\n", 
        	struct_dentry->d_name.name,
        	struct_dentry->d_parent->d_name.name,
        	struct_dentry->d_inode->i_ino
    	);
    } else {
        seq_printf(sf, "dentry not found\n");
    }

    seq_printf(sf, "=============PCI_DEV=============\n");
    seq_printf(sf, "%10s%10s%10s%10s\n", "devfn", "vendor", "device", "class");
    while (struct_pci_dev) {
        seq_printf(sf,	"%10hu%10hu%10hu%10hu\n",
                          struct_pci_dev->devfn,
                          struct_pci_dev->vendor,
                          struct_pci_dev->device,
                          struct_pci_dev->class);

        struct_pci_dev = kmod_get_pci_device(struct_pci_dev);
    }
    return 0;
}

static int kmod_open(
        struct inode *inode,
        struct file *file
) {
    if (!mutex_trylock(&kmod_mutex)) {
        printk(KERN_INFO "can't lock file");
        return -EBUSY;
    }
    printk(KERN_INFO "file is locked by module");
    return single_open(file, kmod_read, inode->i_private);
}

static ssize_t kmod_args_write(struct file* ptr_file, const char __user* buffer, size_t length, loff_t* ptr_offset);

static int kmod_release(
        struct inode *inode,
        struct file *filp
) {
    mutex_unlock(&kmod_mutex);
    printk(KERN_INFO "file is unlocked by module");
    return 0;
}

static struct file_operations kmod_args_ops = {
        .owner   = THIS_MODULE,
        .read    = seq_read,
        .write   = kmod_args_write,
        .open    = kmod_open,
        .release = kmod_release,
};

static unsigned int vendor = PCI_ANY_ID, device = PCI_ANY_ID;
static char pathname[128];

struct pci_dev *kmod_get_pci_device(struct pci_dev *from) {
	return pci_get_device(vendor, device, from);
}

struct dentry *kmod_get_dentry() {
    struct path path;

    int ret = kern_path(
            pathname,
            LOOKUP_FOLLOW,
            &path
    );
    if (ret) {
        return ((void*)0);
    } else {
        return path.dentry;
    }
}

void set_result() {
    struct_pci_dev = kmod_get_pci_device(NULL);
	struct_dentry = kmod_get_dentry();
}

static ssize_t kmod_args_write(
        struct file* ptr_file,
        const char __user* buffer,
        size_t length,
        loff_t* ptr_offset
) {
    printk(KERN_INFO "kmod: get params\n");
    char kbuf[BUFFER_SIZE];

    if (*ptr_offset > 0 || length > BUFFER_SIZE) {
        return -EFAULT;
    }

    if (copy_from_user(kbuf, buffer, length)) {
        return -EFAULT;
    }
    int v, d;
    sscanf(kbuf, "%s %d %d", pathname, &v, &d);
    if (v != -1) vendor = (unsigned int) v;
    if (d != -1) device = (unsigned int) d;

    printk(KERN_INFO "kmod: path: %s, vendor %d, device %d.\n\n",
           pathname, vendor, device);
    ssize_t count = strlen(kbuf);
    *ptr_offset = count;
    return count;
}

static int __init kmod_init(void) {
    printk(KERN_INFO "kmod: module loaded =)\n");
    kmod_root = debugfs_create_dir("kmod", NULL);
    kmod_args_file = debugfs_create_file("kmod_args", 0666, kmod_root, NULL, &kmod_args_ops);
    kmod_result_file = debugfs_create_file("kmod_result", 0666, kmod_root, NULL, &kmod_args_ops);
    return 0;
}

static void __exit kmod_exit(void) {
    debugfs_remove_recursive(kmod_root);
    printk(KERN_INFO "kmod: module unloaded\n");
    mutex_destroy(&kmod_mutex);
}
module_init(kmod_init);
module_exit(kmod_exit);

