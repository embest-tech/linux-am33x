/*
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/input-polldev.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <sound/core.h>
#include <sound/jack.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <trace/events/asoc.h>

#include "ch7026.h"

#define VGA_LCD_480_272  (1)
#define VGA_LCD_640_480  (2)

#define VGA_LCD_RESOLUTION  VGA_LCD_640_480

static unsigned char CH7026_VGA_RegTable[][2] = {
//Attention Please:
//	The following register setting is only the basic configuration, you may have to refer to appendixes of the programming guide document (CH7025(26)B Programming Guide Rev2.03.pdf or later version) to improve the image quality and adjust features as you required...
//{ Reg ,  Val },
#if (VGA_LCD_RESOLUTION == VGA_LCD_480_272)
{ 0x02, 0x01 },
{ 0x02, 0x03 },
{ 0x03, 0x00 },
{ 0x04, 0x39 },
{ 0x08, 0x08 },
{ 0x09, 0x80 },
{ 0x0D, 0x08 },
{ 0x0F, 0x11 },
{ 0x10, 0xE0 },
{ 0x11, 0x0D },
{ 0x12, 0x40 },
{ 0x13, 0x02 },
{ 0x14, 0x29 },
{ 0x15, 0x09 },
{ 0x16, 0x10 },
{ 0x17, 0x1E },
{ 0x19, 0x02 },
{ 0x1A, 0x0A },
{ 0x1B, 0x23 },
{ 0x1C, 0x20 },
{ 0x1D, 0x20 },
{ 0x1F, 0x28 },
{ 0x20, 0x80 },
{ 0x21, 0x12 },
{ 0x22, 0x58 },
{ 0x23, 0x74 },
{ 0x25, 0x01 },
{ 0x26, 0x04 },
{ 0x37, 0x20 },
{ 0x39, 0x20 },
{ 0x3B, 0x20 },
{ 0x41, 0xA2 },
{ 0x4D, 0x03 },
{ 0x4E, 0x0F },
{ 0x4F, 0x8E },
{ 0x50, 0x92 },
{ 0x51, 0x51 },
{ 0x52, 0x12 },
{ 0x53, 0x13 },
{ 0x55, 0xE5 },
{ 0x5E, 0x80 },
{ 0x77, 0x03 },
{ 0x7D, 0x62 },
{ 0x04, 0x38 },
{ 0x06, 0x71 },

/*
NOTE: The following five repeated sentences are used here to wait memory initial complete, please don't remove...(you could refer to Appendix A of programming guide document (CH7025(26)B Programming Guide Rev2.03.pdf or later version) for detailed information about memory initialization!
*/
{ 0x03, 0x00 },
{ 0x03, 0x00 },
{ 0x03, 0x00 },
{ 0x03, 0x00 },
{ 0x03, 0x00 },

{ 0x06, 0x70 },
{ 0x02, 0x02 },
{ 0x02, 0x03 },
{ 0x04, 0x00 },
#elif (VGA_LCD_RESOLUTION == VGA_LCD_640_480)
{ 0x02, 0x01 },
{ 0x02, 0x03 },
{ 0x03, 0x00 },
{ 0x04, 0x39 },
{ 0x08, 0x08 },
{ 0x09, 0x80 },
{ 0x0D, 0x88 },
{ 0x0F, 0x12 },
{ 0x10, 0x80 },
{ 0x11, 0xDA },
{ 0x12, 0x40 },
{ 0x13, 0x0A },
{ 0x15, 0x11 },
{ 0x16, 0xE0 },
{ 0x17, 0x0D },
{ 0x19, 0x0A },
{ 0x1B, 0x23 },
{ 0x1C, 0x20 },
{ 0x1D, 0x20 },
{ 0x1F, 0x28 },
{ 0x20, 0x80 },
{ 0x21, 0x12 },
{ 0x22, 0x58 },
{ 0x23, 0x74 },
{ 0x25, 0x01 },
{ 0x26, 0x04 },
{ 0x37, 0x20 },
{ 0x39, 0x20 },
{ 0x3B, 0x20 },
{ 0x41, 0xA2 },
{ 0x4D, 0x03 },
{ 0x4E, 0x0F },
{ 0x4F, 0x8E },
{ 0x50, 0x92 },
{ 0x51, 0x51 },
{ 0x52, 0x12 },
{ 0x53, 0x13 },
{ 0x55, 0xE5 },
{ 0x5E, 0x80 },
{ 0x77, 0x03 },
{ 0x7D, 0x62 },
{ 0x04, 0x38 },
{ 0x06, 0x71 },

/*
NOTE: The following five repeated sentences are used here to wait memory initial complete, please don't remove...(you could refer to Appendix A of programming guide document (CH7025(26)B Programming Guide Rev2.03.pdf or later version) for detailed information about memory initialization!
*/
{ 0x03, 0x00 },
{ 0x03, 0x00 },
{ 0x03, 0x00 },
{ 0x03, 0x00 },
{ 0x03, 0x00 },

{ 0x06, 0x70 },
{ 0x02, 0x02 },
{ 0x02, 0x03 },
{ 0x04, 0x00 },
#endif
};

static struct i2c_client *g_client_ch7026;


static int ch7026_write_reg(unsigned int index, unsigned char value)
{
	int result;
	result = i2c_smbus_write_byte_data(g_client_ch7026, index, value);
	return result;
}

static unsigned char ch7026_read_reg(unsigned int index)
{
	unsigned char result;
	result = i2c_smbus_read_byte_data(g_client_ch7026, index);
	return result;
}

static int ch7026_init_reg(void)
{
	int result;
	int i;

	for(i=0;i<(sizeof(CH7026_VGA_RegTable)/sizeof(CH7026_VGA_RegTable[0]));i++){
		result += ch7026_write_reg(CH7026_VGA_RegTable[i][0],CH7026_VGA_RegTable[i][1]);
	}
	if(result)
		return -1;
	else
		return 0;
}

// init mux pca9543a
static int pca9543a_reg_write(struct i2c_client *client, u8 val)
{
	struct i2c_adapter *adap = client->adapter;
	int ret;

	if (adap->algo->master_xfer) {
		struct i2c_msg msg;
		char buf[1];
		buf[0] = val;
		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 1;
		msg.buf = buf;
		ret = __i2c_transfer(adap, &msg, 1);
	}
	return ret;
}

static int  pca9543a_reg_read(struct i2c_client *client)
{
	struct i2c_adapter *adap = client->adapter;
	int ret;
	u8 val;

	if (adap->algo->master_xfer) {
		//printk("%s %d \n",__func__,__LINE__);
		struct i2c_msg msg[1] = {
			{
				.addr = client->addr,
				.flags = I2C_M_RD,
				.len = 1,
				.buf = &val
			},
		};
		ret = __i2c_transfer(adap, msg, 1);
		if (ret == 1)
			ret = val;
		else if (ret >= 0)
			ret = -EIO;
	}
	return ret;
}


// the mux is PCA9543A
#define PCA9543A_ADDR  (0x70)
#define PCA9543A_CH_0   ((unsigned char)0x1)
#define PCA9543A_CH_1   ((unsigned char)0x2)

static int enable_mux_ch7026(void)
{
	int ret;
	unsigned char pca9543a_crtl_reg;
	struct i2c_client client_pac9543;

	// ch7026 connected to ch1 of pca9543a
	client_pac9543 = * g_client_ch7026;
	client_pac9543.addr = PCA9543A_ADDR;

	pca9543a_crtl_reg = pca9543a_reg_read(&client_pac9543);
	pca9543a_crtl_reg |=  PCA9543A_CH_0 | PCA9543A_CH_1;

	ret = pca9543a_reg_write(&client_pac9543,pca9543a_crtl_reg);
	return ret;
}

static int ch7026_hard_reset(void)
{

	struct gpio_desc *pch7026_reset_pin;
	int err;

	pch7026_reset_pin = devm_gpiod_get(&g_client_ch7026->dev, NULL);
	if (IS_ERR(pch7026_reset_pin))
		return PTR_ERR(pch7026_reset_pin);

	// pull down the reset pin
	err = gpiod_direction_output(pch7026_reset_pin, 0);
	if (err)
		return err;

	gpiod_set_value_cansleep(pch7026_reset_pin, 0);
	mdelay(10);
	// pull up the reset pin
	gpiod_set_value_cansleep(pch7026_reset_pin, 1);
	return 0;
}



static int  ch7026_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	int ret;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	printk(KERN_INFO "ch7026_probe........................\n");
	// get the clinet
	g_client_ch7026 = client;
	//set the reset pin
	ret = ch7026_hard_reset();
	//enable mux
	enable_mux_ch7026();
	//set the resisters
	ch7026_init_reg();
	return 0;
}

static int  ch7026_remove(struct i2c_client *client)
{
	return 0;
}



static const struct of_device_id ch7026_of_match[] = {
	{ .compatible = "chrontel,ch7026", },
};

static const struct i2c_device_id ch7026_id[] = {
	{ CH7026_DRV_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ch7026_id);

static struct i2c_driver ch7026_driver = {
	.driver = {
		.name	= CH7026_DRV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = ch7026_of_match,
	},
	.probe	= ch7026_probe,
	.remove	= ch7026_remove,
	.id_table = ch7026_id,
};


module_i2c_driver(ch7026_driver);

MODULE_AUTHOR("david <david.fu@embest-tech.com>");
MODULE_DESCRIPTION("ch7026 VGA driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

