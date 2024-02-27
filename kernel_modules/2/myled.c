#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of.h>

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

static int myled_driver_probe(struct platform_device *pdev)
{
	if (IS_ERR(gpio_desc = gpiod_get(&pdev->dev, "myled", GPIOD_OUT_LOW))) {
		dev_err(&pdev->dev, "error getting gpio_desc\n");
		return -1;
	}

	if (of_property_read_string(pdev->dev.of_node, "label", &led_cdev.name) < 0) {
		dev_err(&pdev->dev, "error reading label\n");
		return -1;
	}

	led_cdev.brightness_set = myled_on_change_function;
	if (devm_led_classdev_register(&pdev->dev, &led_cdev) < 0) {
		dev_err(&pdev->dev, "error registering led class device\n");
		return -1;
	}

	gpiod_direction_output(gpio_desc, 0);

	return 0;
}

static int myled_driver_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id of_drvled_match[] = {
	{ .compatible = "breadboard,myled" },
	{},
};

static struct platform_driver drvled_driver = {
	.driver = {
		.name	= "myled",
		.owner	= THIS_MODULE,
		.of_match_table = of_drvled_match,
	},
	.probe		= myled_driver_probe,
	.remove		= myled_driver_remove,
};

module_platform_driver(drvled_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Josue David Hernandez Gutierrez <hernandez.josued@gmail.com>");
MODULE_VERSION("0.1");
MODULE_INFO(intree, "Y");
