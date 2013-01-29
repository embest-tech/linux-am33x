/*
 * character device wrapper for generic gpio layer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA02111-1307USA
 *
 * Feedback, Bugs...  blogic@openwrt.org
 *
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/atomic.h>
#include <linux/init.h>
#include <linux/genhd.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))
#define UART1_RTS_CTL GPIO_TO_PIN(0, 13)
static int gpio0_13_state;

static ssize_t io_ctl_read(struct file *filp, char *buf,size_t count,loff_t *f_ops)
{
	return count;
}

static ssize_t io_ctl_write(struct file *filp,const char *buf,size_t count,loff_t *f_ops)
{
	return count;
}

static int io_ctl_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static int io_ctl_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int io_ctl_close(struct inode * inode, struct file * file)
{
	return 0;
}

static ssize_t gpio0_13_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", gpio0_13_state);
}

static ssize_t gpio0_13_state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
        gpio0_13_state = simple_strtoul(buf, NULL, 10);

	printk("--------gpio0_13 set to %d\n", gpio0_13_state);
        gpio_direction_output(UART1_RTS_CTL, gpio0_13_state);

        return size;
}

struct file_operations io_ctl_fops = {
	.read		= io_ctl_read,
	.write		= io_ctl_write,	
	.unlocked_ioctl	= io_ctl_ioctl,
	.open		= io_ctl_open,
	.release	= io_ctl_close,
};

static struct miscdevice io_ctl_dev = {
        .minor         = MISC_DYNAMIC_MINOR,
        .name         = "io_ctl",                   
        .fops         = &io_ctl_fops,
};

static DEVICE_ATTR(gpio0_13_state, 0644, gpio0_13_state_show, gpio0_13_state_store);

static struct attribute *io_ctl_attributes[] = {
	&dev_attr_gpio0_13_state.attr,
        NULL,
};

static struct attribute_group io_ctl_attr_group = {
        .attrs = io_ctl_attributes,
};

static int __init io_ctl_dev_init(void)
{
	int ret;
	
	/*printk("--------io_ctl_dev_init\n");*/

	ret = gpio_request(UART1_RTS_CTL, "gpio0_13");
	if(ret < 0)
		printk("--------request gpio %d fail!\n", 136);
	
  
	ret = misc_register(&io_ctl_dev);
	if(ret){
		printk(KERN_ERR "misc_register failed\n");
		return ret;
	}

	ret = sysfs_create_group(&io_ctl_dev.this_device->kobj, &io_ctl_attr_group);
        if (ret){
		printk(KERN_ERR "creat attr file failed\n");
		misc_deregister(&io_ctl_dev);
		return ret;
	}

	return 0;
}

static void __exit io_ctl_dev_exit(void)
{
	sysfs_remove_group(&io_ctl_dev.this_device->kobj, &io_ctl_attr_group);
	misc_deregister(&io_ctl_dev);
}

module_init (io_ctl_dev_init);
module_exit (io_ctl_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embest");
MODULE_DESCRIPTION("Character device for power ctl");
