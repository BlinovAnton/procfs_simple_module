#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

/* bug with echo -e "...\n" > file - it is refreshing a file*/

/*ks - kernel space, us - user space*/

MODULE_LICENSE("GPL");
MODULE_LICENCE("GPL v2");

static size_t ks_len = 0;
static loff_t g_offset = 0;
static char *end_combo = "end ";
static const char *proc_dir_name = "anton";
static char *ks_buff = NULL, *ks_buff_temp = NULL;
static const char *proc_file_name = "ant_proc_file";
static struct proc_dir_entry *pde_file = NULL, *pde_dir = NULL;
static ssize_t proc_read (struct file *, char *, size_t, loff_t *);
static ssize_t proc_write (struct file *, const char *, size_t, loff_t *);
static struct file_operations file_ops = {
    .read = proc_read,
    .write = proc_write,
};

static ssize_t proc_read (struct file *file, char *us_buff, size_t us_len, loff_t *offset) {
    int read_res;
    if (ks_buff == NULL) {
	//printk(KERN_INFO "Nothing to read\n");
	return 0;
    } else {
	//printk(KERN_INFO "R: off %d %d us %d\n", (int)*offset, (int)g_offset, (int)us_len);
	read_res = simple_read_from_buffer(us_buff, us_len, offset, ks_buff, ks_len);
	//printk(KERN_INFO "R: off %d %d us %d\n", (int)*offset, (int)g_offset, (int)us_len);
	return read_res;
    }
}

static ssize_t proc_write (struct file *file, const char *us_buff, size_t us_len, loff_t *offset) {
    int write_res = 0;
    /* start "refreshing" block */
    if (!strncmp(end_combo, us_buff, us_len - 1)) {
	ks_len = 0;
	g_offset = 0;
	if (!ks_buff) {
	    kfree(ks_buff);
	    ks_buff_temp = NULL;
	    //printk(KERN_INFO "file refreshed\n");
	}
	return strlen(end_combo);
    }
    /* end "refreshing" block */

    /* start "write/add" block */
    ks_len = ks_len + us_len;
    ks_buff_temp = krealloc(ks_buff, ks_len, GFP_KERNEL);
    if (ks_buff_temp == NULL) {
	printk(KERN_WARNING "kmalloc can't alloc memory, device_write fault\n");
	return -EFAULT;
    } else {
	ks_buff = ks_buff_temp;
	//printk(KERN_INFO "W: off %d %d us %d\n", (int)*offset, (int)g_offset, (int)us_len);
	write_res = simple_write_to_buffer(ks_buff, ks_len, &g_offset, us_buff, us_len);
	//printk(KERN_INFO "W: off %d %d us %d\n", (int)*offset, (int)g_offset, (int)us_len);
	return write_res;
    }
    /* end "write/add" block */
}

static int __init proc_init (void) {
	pde_dir = proc_mkdir(proc_dir_name, NULL);
	if (!pde_dir) {
	    printk(KERN_WARNING "Failed to register dir /proc/%s/\n", proc_dir_name);
	    return -EFAULT;
	}
	pde_file = proc_create(proc_file_name, 0644, pde_dir, &file_ops);
	if (!pde_file) {
	    printk(KERN_WARNING "Failed to register /proc/%s/%s\n", proc_dir_name, proc_file_name);
	    return -EFAULT;
	}
	printk(KERN_INFO "Successfully registered /proc/%s/%s\n", proc_dir_name, proc_file_name);
	return 0;
}

static void __exit proc_exit (void) {
	if (ks_buff != NULL) {
	    kfree(ks_buff);
	}
	proc_remove(pde_dir);
	printk(KERN_INFO "Unregistered /proc/%s/*\n", proc_dir_name);
}

module_init (proc_init);
module_exit (proc_exit);