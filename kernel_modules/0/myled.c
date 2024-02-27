#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/err.h>

#define GPIO_PIN (21)

#define SEND_BUF_LEN 2
#define REC_BUF_LEN 2

dev_t dev = 0;
static struct class *dev_class;
static struct cdev myled_cdev;
static struct gpio_desc *gpio_desc;

static ssize_t myled_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
	uint8_t state = 0;
	uint8_t send_buf[SEND_BUF_LEN] = { 0 };
	state = gpiod_get_value(gpio_desc);
 
	if (state) send_buf[0] = '1';
	else send_buf[0] = '0';

	if (copy_to_user(buf, send_buf, SEND_BUF_LEN) > 0)
		return -1;
  
	return 0;
}

static ssize_t myled_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
	uint8_t rec_buf[REC_BUF_LEN] = { 0 };

	if (len > REC_BUF_LEN)
		return -1;

  
	if (copy_from_user(rec_buf, buf, len) > 0)
		return -1;
  
	if (rec_buf[0] == '1')		gpiod_set_value(gpio_desc, 1);
	else if (rec_buf[0] == '0')	gpiod_set_value(gpio_desc, 0);
	else				return -EINVAL;

	return len;
}

static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .read           = myled_read,
  .write          = myled_write,
};

static int __init myled_driver_init(void)
{
	if (alloc_chrdev_region(&dev, 0, 1, "myled") < 0)
		return -1;

	cdev_init(&myled_cdev, &fops);

	if (cdev_add(&myled_cdev, dev, 1) < 0)
		goto CHRDEV_UNREG;

	if (IS_ERR(dev_class = class_create(THIS_MODULE, "myled_class")))
		goto CDEV_DEL;

	if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "myled")))
		goto CLASS_DESTROY;

	if (gpio_request(GPIO_PIN, "myled") < 0)
		goto DEVICE_DESTROY;

	gpio_desc = gpio_to_desc(GPIO_PIN);
	gpiod_direction_output(gpio_desc, 0);

	return 0;
DEVICE_DESTROY:
	device_destroy(dev_class, dev);
CLASS_DESTROY:
	class_destroy(dev_class);
CDEV_DEL:
	cdev_del(&myled_cdev);
CHRDEV_UNREG:
	unregister_chrdev_region(dev, 1);
	return -1;
}

static void __exit myled_driver_exit(void)
{
	gpio_free(GPIO_PIN);
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&myled_cdev);
	unregister_chrdev_region(dev, 1);
}

module_init(myled_driver_init);
module_exit(myled_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Josue David Hernandez Gutierrez <hernandez.josued@gmail.com>");
MODULE_VERSION("0.1");
