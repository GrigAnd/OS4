#ifndef NFS_ENTRYPOINT_H
#define NFS_ENTRYPOINT_H

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

struct dentry *networkfs_lookup(struct inode *, struct dentry *, unsigned int);

int networkfs_iterate(struct file *, struct dir_context *);

struct file_operations networkfs_dir_ops = {
    .iterate = networkfs_iterate,
};

struct dentry *networkfs_lookup(struct inode *, struct dentry *, unsigned int);

struct inode_operations networkfs_inode_ops = {
    .lookup = networkfs_lookup,
};

struct inode *networkfs_get_inode(struct super_block *, const struct inode *, umode_t, int);

int networkfs_fill_super(struct super_block *, void *, int);

struct dentry *networkfs_mount(struct file_system_type *, int, const char *, void *);

void networkfs_kill_sb(struct super_block *);

struct file_system_type networkfs_fs_type = {
    .name = "networkfs",
    .mount = networkfs_mount,
    .kill_sb = networkfs_kill_sb,
};

int networkfs_init(void);
void networkfs_exit(void);

#endif
