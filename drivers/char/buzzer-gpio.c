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


struct gpio_buzzer {
	struct gpio_desc	*desc;
	int			beeping;
};

static ssize_t buzzer_ctl_read(struct file *filp, char *buf,size_t count,loff_t *f_ops)
{
	return count;
}

static ssize_t buzzer_ctl_write(struct file *filp,const char *buf,size_t count,loff_t *f_ops)
{
	return count;
}

static long buzzer_ctl_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static int buzzer_ctl_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int buzzer_ctl_close(struct inode * inode, struct file * file)
{
	return 0;
}

static ssize_t buzzer_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct gpio_buzzer *pbuzzer = dev_get_drvdata(dev);
        return sprintf(buf, "%u\n", pbuzzer->beeping);
}

static ssize_t buzzer_state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
        struct gpio_buzzer *pbuzzer = dev_get_drvdata(dev);
        pbuzzer->beeping = simple_strtoul(buf, NULL, 10);

        gpiod_set_value_cansleep(pbuzzer->desc, pbuzzer->beeping);

        return size;
}

struct file_operations buzzer_ctl_fops = {
	.read		= buzzer_ctl_read,
	.write		= buzzer_ctl_write,	
	.unlocked_ioctl	= buzzer_ctl_ioctl,
	.open		= buzzer_ctl_open,
	.release	= buzzer_ctl_close,
};

static struct miscdevice buzzer_ctl_dev = {
        .minor         = MISC_DYNAMIC_MINOR,
        .name         = "buzzer_ctl",                   
        .fops         = &buzzer_ctl_fops,
};

static DEVICE_ATTR(state, 0644, buzzer_state_show, buzzer_state_store);

static struct attribute *buzzer_ctl_attributes[] = {
	&dev_attr_state.attr,
        NULL,
};

static struct attribute_group buzzer_ctl_attr_group = {
        .attrs = buzzer_ctl_attributes,
};

static int gpio_buzzer_probe(struct platform_device *pdev)
{
        struct gpio_buzzer *buzzer;
        int err;
        buzzer = devm_kzalloc(&pdev->dev, sizeof(*buzzer), GFP_KERNEL);
        if (!buzzer)
        	return -ENOMEM;

        buzzer->desc = devm_gpiod_get(&pdev->dev, NULL);
        if (IS_ERR(buzzer->desc))
        	return PTR_ERR(buzzer->desc);

        buzzer->beeping = 0;

        err = gpiod_direction_output(buzzer->desc, 0);
        if (err)
        	return err;

        err = misc_register(&buzzer_ctl_dev);
        if(err){
        	printk(KERN_ERR "misc_register failed\n");
        	return err;
        }

        err = sysfs_create_group(&buzzer_ctl_dev.this_device->kobj, &buzzer_ctl_attr_group);
        if (err){
        	printk(KERN_ERR "creat attr file failed\n");
        	misc_deregister(&buzzer_ctl_dev);
        	return err;
        }    
        platform_set_drvdata(pdev, buzzer);
        dev_set_drvdata(buzzer_ctl_dev.this_device, buzzer);
        return 0;
}

static int  gpio_buzzer_remove(struct platform_device *pdev)
{
	struct gpio_buzzer *buzzer = dev_get_drvdata(&pdev->dev);

    sysfs_remove_group(&buzzer_ctl_dev.this_device->kobj, &buzzer_ctl_attr_group);
	misc_deregister(&buzzer_ctl_dev);

	dev_set_drvdata(&pdev->dev, NULL);
    dev_set_drvdata(buzzer_ctl_dev.this_device, NULL);
	kfree(buzzer);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id gpio_buzzer_of_match[] = {
	{ .compatible = "gpio_buzzer", },
	{ }
};
MODULE_DEVICE_TABLE(of, gpio_buzzer_of_match);
#endif

static struct platform_driver gpio_buzzer_platform_driver = {
	.driver	= {
		.name		= "gpio_buzzer",
		.of_match_table	= of_match_ptr(gpio_buzzer_of_match),
	},
	.probe	= gpio_buzzer_probe,
    .remove		= (gpio_buzzer_remove),
};
module_platform_driver(gpio_buzzer_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embest <rongdong@embest-tech.com>");
MODULE_DESCRIPTION("Generic GPIO buzzer driver");
