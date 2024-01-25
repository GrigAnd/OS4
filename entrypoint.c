#include "entrypoint.h"

#include <linux/list.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GrigAnd");
MODULE_DESCRIPTION("Hello world");
MODULE_VERSION("1.0");

static int inode_counter = 101;

struct entry {
    char name[128];
    unsigned char ftype;
    ino_t ino;
    ino_t parent_ino;
    struct list_head list;
};

struct list_head entry_list;

void add_entry(const char *name, unsigned char ftype, ino_t ino, ino_t parent_ino) {
    struct entry *new_entry;
    new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if (new_entry) {
        strncpy(new_entry->name, name, sizeof(new_entry->name));
        new_entry->ftype = ftype;
        new_entry->ino = ino;
        new_entry->parent_ino = parent_ino;
        INIT_LIST_HEAD(&new_entry->list);
        list_add_tail(&new_entry->list, &entry_list);
    }
}

void remove_entry(struct entry *entry) {
    list_del(&entry->list);
    kfree(entry);
}

void init_entries(void) {
    INIT_LIST_HEAD(&entry_list);
    add_entry("test.txt", DT_REG, inode_counter++, 100);
    add_entry("test2.txt", DT_REG, inode_counter++, 100);
    add_entry("dir", DT_DIR, inode_counter++, 100);
    printk(KERN_INFO "entrys added\n");
}

int networkfs_create(struct user_namespace *uns, struct inode *parent_inode, struct dentry *child_dentry, umode_t mode, bool b) {
    ino_t root;
    struct inode *inode;
    const char *name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    inode = networkfs_get_inode(parent_inode->i_sb, NULL, S_IFREG, inode_counter++);
    inode->i_mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
    d_add(child_dentry, inode);

    add_entry(name, DT_REG, inode_counter++, root);

    return 0;
}

int networkfs_mkdir(struct user_namespace *uns, struct inode *parent_inode, struct dentry *child_dentry, umode_t mode) {
    ino_t root;
    struct inode *inode;
    const char *name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    inode = networkfs_get_inode(parent_inode->i_sb, NULL, S_IFDIR, inode_counter++);
    inode->i_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
    d_add(child_dentry, inode);

    add_entry(name, DT_DIR, inode_counter++, root);

    return 0;
}

int networkfs_rmdir(struct inode *parent_inode, struct dentry *child_dentry) {
    ino_t root;
    const char *name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    struct entry *entry;
    list_for_each_entry(entry, &entry_list, list) {
        if (entry->parent_ino == root && !strcmp(name, entry->name)) {
            remove_entry(entry);
            break;
        }
    }

    return 0;
}

int networkfs_unlink(struct inode *parent_inode, struct dentry *child_dentry) {
    ino_t root;
    const char *name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    struct entry *entry;
    list_for_each_entry(entry, &entry_list, list) {
        if (entry->parent_ino == root && !strcmp(name, entry->name)) {
            remove_entry(entry);
            break;
        }
    }

    return 0;
}

struct dentry *networkfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flag) {
    ino_t root;
    struct inode *inode;
    const char *name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    struct entry *entry;
    list_for_each_entry(entry, &entry_list, list) {
        if (entry->parent_ino == root && !strcmp(name, entry->name)) {
            inode = networkfs_get_inode(parent_inode->i_sb, NULL, entry->ftype, entry->ino);
            inode->i_mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
            d_add(child_dentry, inode);
            break;
        }
    }

    return NULL;
}

int networkfs_iterate(struct file *filp, struct dir_context *ctx) {
    struct entry *entry;
    struct dentry *dentry;
    struct inode *inode;
    unsigned long offset;
    int stored;
    ino_t ino;
    dentry = filp->f_path.dentry;
    inode = dentry->d_inode;
    offset = filp->f_pos;
    stored = 0;
    ino = inode->i_ino;

    if (offset >= 1) {
        return 0;
    }

    printk(KERN_INFO "ino: %lu, offset: %lu\n", (unsigned long)ino, offset);
    dir_emit(ctx, ".", 1, ino, DT_DIR);
    ctx->pos += 1;
    stored += 1;
    offset++;
    dir_emit(ctx, "..", 2, dentry->d_parent->d_inode->i_ino, DT_DIR);
    ctx->pos += 1;
    stored += 1;
    offset++;
    list_for_each_entry(entry, &entry_list, list) {
        if (entry->parent_ino == ino) {
            printk(KERN_INFO "entry: %s, %d, %lu, %lu\n", entry->name, entry->ftype, (unsigned long)entry->ino, (unsigned long)entry->parent_ino);
            if (dir_emit(ctx, entry->name, strlen(entry->name), entry->ino, entry->ftype)) {
                ctx->pos += 1;
                stored += 1;
                offset++;
            }
        }
    }

    return stored;
}

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

int networkfs_init(void) {
    printk(KERN_INFO "Init\n");
    inode_counter = 101;
    int ret = register_filesystem(&networkfs_fs_type);
    init_entries();

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
