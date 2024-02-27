#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/err.h>

#define GPIO_PIN (21)

static struct gpio_desc *gpio_desc;
struct led_classdev led_cdev;

static void myled_on_change_function(struct led_classdev *led_cdev,
				     enum led_brightness brightness)
{
	if (brightness)
		gpiod_set_value(gpio_desc, 1);
	else
		gpiod_set_value(gpio_desc, 0);
}

static int __init myled_driver_init(void)
{
	if (gpio_request(GPIO_PIN, "myled") < 0)
		return -1;

	gpio_desc = gpio_to_desc(GPIO_PIN);

	led_cdev.name = "myled:red:user";
	led_cdev.brightness_set = myled_on_change_function;

	if (led_classdev_register(NULL, &led_cdev) < 0)
		goto GPIO_FREE;

	gpiod_direction_output(gpio_desc, 0);

	return 0;
GPIO_FREE:
	gpio_free(GPIO_PIN);
	return -1;
}

static void __exit myled_driver_exit(void)
{
	led_classdev_unregister(&led_cdev);
	gpio_free(GPIO_PIN);
}

module_init(myled_driver_init);
module_exit(myled_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Josue David Hernandez Gutierrez <hernandez.josued@gmail.com>");
MODULE_VERSION("0.1");
