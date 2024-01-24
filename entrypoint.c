#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GrigAnd");
MODULE_DESCRIPTION("Hello world");
MODULE_VERSION("1.0");

#define MAX_ENTRIES 10

int networkfs_iterate(struct file *filp, struct dir_context
                                             *ctx) {
    char fsname[10];
    struct dentry *dentry;
    struct inode *inode;
    unsigned long offset;
    int stored;
    unsigned char ftype;
    ino_t ino;
    ino_t dino;
    dentry = filp->f_path.dentry;
    inode = dentry->d_inode;
    offset = filp->f_pos;
    stored = 0;
    ino = inode->i_ino;

    while (true) {
        if (ino == 100) {
            if (offset == 0) {
                strcpy(fsname, ".");
                ftype = DT_DIR;
                dino = ino;
            } else if (offset == 1) {
                strcpy(fsname, "..");
                ftype = DT_DIR;
                dino = dentry->d_parent->d_inode->i_ino;
            } else if (offset == 2) {
                strcpy(fsname, "test.txt");
                ftype = DT_REG;
                dino = 101;
            } else {
                return stored;
            }
        }

        if (dir_emit(ctx, fsname, strlen(fsname), dino, ftype)) {
            ctx->pos += 1;
            stored += 1;  // Increment stored if the entry is successfully emitted
        }

        offset++;
    }

return stored;
}

struct file_operations networkfs_dir_ops =
    {
        .iterate = networkfs_iterate,
};

struct dentry *networkfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flag) {
    return NULL;
}

struct inode_operations networkfs_inode_ops =
    {
        .lookup = networkfs_lookup,
};

struct inode *networkfs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, int i_ino) {
    struct inode *inode;
    inode = new_inode(sb);
    inode->i_ino = i_ino;
    inode->i_op = &networkfs_inode_ops;
    inode->i_fop = &networkfs_dir_ops;
    if (inode != NULL) {
        inode_init_owner(&init_user_ns, inode, dir, mode);
    }
    return inode;
}

int networkfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct inode *inode;
    inode = networkfs_get_inode(sb, NULL, S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO, 100);
    sb->s_root = d_make_root(inode);
    if (sb->s_root == NULL) {
        return -ENOMEM;
    }
    printk(KERN_INFO "return 0\n");
    return 0;
}

struct dentry *networkfs_mount(struct file_system_type *fs_type, int flags, const char *token, void *data) {
    struct dentry *ret;
    ret = mount_nodev(fs_type, flags, data, networkfs_fill_super);
    if (ret == NULL) {
        printk(KERN_ERR "Can't mount file system");
    } else {
        printk(KERN_INFO "Mounted successfuly");
    }
    return ret;
}

void networkfs_kill_sb(struct super_block *sb) {
    printk(KERN_INFO "networkfs super block is destroyed. Unmount successfully.\n");
}

struct file_system_type networkfs_fs_type =
    {
        .name = "networkfs",
        .mount = networkfs_mount,
        .kill_sb = networkfs_kill_sb};

int networkfs_init(void) {
    printk(KERN_INFO "Init\n");
    int ret = register_filesystem(&networkfs_fs_type);
    if (ret != 0) {
        printk(KERN_ERR "Can't register file system");
    } else {
        printk(KERN_INFO "Registered successfuly");
    }
    return ret;
}

void networkfs_exit(void) {
    printk(KERN_INFO "Exit\n");
    int ret = unregister_filesystem(&networkfs_fs_type);
    if (ret != 0) {
        printk(KERN_ERR "Can't unregister file system");
    } else {
        printk(KERN_INFO "Unregistered successfuly");
    }
}

module_init(networkfs_init);
module_exit(networkfs_exit);
