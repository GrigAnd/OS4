#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

int networkfs_init(void) {
    printk(KERN_INFO "Hello, World!\n");
    return 0;
}

void networkfs_exit(void) {
    printk(KERN_INFO "Exit\n");
}
module_init(networkfs_init);
module_exit(networkfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GrigAnd");
MODULE_DESCRIPTION("Hello world");
MODULE_VERSION("1.0");
