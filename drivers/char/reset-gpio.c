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
#include <linux/gpio/consumer.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/genhd.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>


struct gpio_reset {
	struct gpio_desc	*desc;
	struct gpio_desc	*wifi_reset;
	int			beeping;
};

static ssize_t reset_ctl_read(struct file *filp, char *buf,size_t count,loff_t *f_ops)
{
	return count;
}

static ssize_t reset_ctl_write(struct file *filp,const char *buf,size_t count,loff_t *f_ops)
{
	return count;
}

static long reset_ctl_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static int reset_ctl_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int reset_ctl_close(struct inode * inode, struct file * file)
{
	return 0;
}

static ssize_t reset_bluetooth_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct gpio_reset *preset = dev_get_drvdata(dev);
        return sprintf(buf, "%u\n", preset->beeping);
}

static ssize_t reset_bluetooth_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
        struct gpio_reset *preset = dev_get_drvdata(dev);
        preset->beeping = simple_strtoul(buf, NULL, 10);

        gpiod_set_value_cansleep(preset->desc, preset->beeping);
	//gpiod_set_value_cansleep(preset->wifi_reset, preset->beeping);

        return size;
}

static ssize_t reset_wifi_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
        struct gpio_reset *preset = dev_get_drvdata(dev);
        preset->beeping = simple_strtoul(buf, NULL, 10);

        //gpiod_set_value_cansleep(preset->desc, preset->beeping);
	gpiod_set_value_cansleep(preset->wifi_reset, preset->beeping);

        return size;
}

struct file_operations reset_ctl_fops = {
	.read		= reset_ctl_read,
	.write		= reset_ctl_write,	
	.unlocked_ioctl	= reset_ctl_ioctl,
	.open		= reset_ctl_open,
	.release	= reset_ctl_close,
};

static struct miscdevice reset_ctl_dev = {
        .minor         = MISC_DYNAMIC_MINOR,
        .name         = "reset_ctl",                   
        .fops         = &reset_ctl_fops,
};

static DEVICE_ATTR(bluetooth, 0644, reset_bluetooth_show, reset_bluetooth_store);
static DEVICE_ATTR(wifi, 0644, reset_bluetooth_show, reset_wifi_store);

static struct attribute *reset_ctl_attributes[] = {
	&dev_attr_bluetooth.attr,
	&dev_attr_wifi.attr,
        NULL,
};

static struct attribute_group reset_ctl_attr_group = {
        .attrs = reset_ctl_attributes,
};

static int gpio_reset_probe(struct platform_device *pdev)
{
        struct gpio_reset *reset;
        int err;
	printk(KERN_ERR "I will reset the wifi bt modules\n");
	//while(1);
        reset = devm_kzalloc(&pdev->dev, sizeof(*reset), GFP_KERNEL);
        if (!reset)
        	return -ENOMEM;

        reset->desc = devm_gpiod_get_optional(&pdev->dev, "bt", GPIOD_OUT_LOW);
        if (IS_ERR(reset->desc))
        	return PTR_ERR(reset->desc);

	reset->wifi_reset = devm_gpiod_get_optional(&pdev->dev, "wifi", GPIOD_OUT_LOW);
        if (IS_ERR(reset->wifi_reset))
        	return PTR_ERR(reset->wifi_reset);
		
        reset->beeping = 1;

        err = gpiod_direction_output(reset->desc, 0);
        if (err)
        	return err;
	err = gpiod_direction_output(reset->wifi_reset, 0);
        if (err)
        	return err;
	mdelay(10);
	err = gpiod_direction_output(reset->desc, 1);
        if (err)
        	return err;
	err = gpiod_direction_output(reset->wifi_reset, 1);
        if (err)
        	return err;

        err = misc_register(&reset_ctl_dev);
        if(err){
        	printk(KERN_ERR "misc_register failed\n");
        	return err;
        }

        err = sysfs_create_group(&reset_ctl_dev.this_device->kobj, &reset_ctl_attr_group);
        if (err){
        	printk(KERN_ERR "creat attr file failed\n");
        	misc_deregister(&reset_ctl_dev);
        	return err;
        }    
        platform_set_drvdata(pdev, reset);
        dev_set_drvdata(reset_ctl_dev.this_device, reset);
        return 0;
}

static int  gpio_reset_remove(struct platform_device *pdev)
{
	struct gpio_reset *reset = dev_get_drvdata(&pdev->dev);

    sysfs_remove_group(&reset_ctl_dev.this_device->kobj, &reset_ctl_attr_group);
	misc_deregister(&reset_ctl_dev);

	dev_set_drvdata(&pdev->dev, NULL);
    dev_set_drvdata(reset_ctl_dev.this_device, NULL);
	kfree(reset);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id gpio_reset_of_match[] = {
	{ .compatible = "gpio_reset", },
	{ }
};
MODULE_DEVICE_TABLE(of, gpio_reset_of_match);
#endif

static struct platform_driver gpio_reset_platform_driver = {
	.driver	= {
		.name		= "gpio_reset",
		.of_match_table	= of_match_ptr(gpio_reset_of_match),
	},
	.probe	= gpio_reset_probe,
    .remove		= (gpio_reset_remove),
};
module_platform_driver(gpio_reset_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embest <rongdong@embest-tech.com>");
MODULE_DESCRIPTION("Generic GPIO reset driver");
