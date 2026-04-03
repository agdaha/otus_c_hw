#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/types.h>

#define DEVICE_NAME "ring_buffer_dev"
#define CLASS_NAME "ring_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("agdaha");
MODULE_DESCRIPTION("Драйвер символьного устройства кольцевой буфер");
MODULE_VERSION("0.1");

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *buffer_class = NULL;
static struct device *buffer_device = NULL;

//=============================================================================
// Кольцевой буфер
static char *ring_buffer;
static size_t buffer_size = 1024;
static size_t head = 0;
static size_t tail = 0;
static bool is_full = false;

static DEFINE_MUTEX(buffer_lock);

static void buffer_write_char(char c)
{
    ring_buffer[head] = c;

    if (is_full)
    {
        tail = (tail + 1) % buffer_size;
    }

    head = (head + 1) % buffer_size;
    is_full = (head == tail);
}

static int buffer_read_char(char *c)
{
    if (!is_full && (head == tail))
        return -1;

    *c = ring_buffer[tail];
    tail = (tail + 1) % buffer_size;
    is_full = false;
    return 0;
}

//=============================================================================
// Операции с файлом /dev/{DEVICE_NAME}
static ssize_t dev_read(struct file *file, char __user *user_buf, size_t len, loff_t *off)
{
    size_t bytes_read = 0;
    char c;

    if (mutex_lock_interruptible(&buffer_lock))
        return -ERESTARTSYS;

    while (bytes_read < len)
    {
        if (buffer_read_char(&c) != 0)
            break;

        if (put_user(c, user_buf + bytes_read))
        {
            mutex_unlock(&buffer_lock);
            return -EFAULT;
        }
        bytes_read++;
    }

    mutex_unlock(&buffer_lock);
    return bytes_read;
}

static ssize_t dev_write(struct file *file, const char __user *user_buf, size_t len, loff_t *off)
{
    size_t i;
    char c;

    if (mutex_lock_interruptible(&buffer_lock))
        return -ERESTARTSYS;

    for (i = 0; i < len; i++)
    {
        if (get_user(c, user_buf + i))
        {
            mutex_unlock(&buffer_lock);
            return -EFAULT;
        }
        buffer_write_char(c);
    }

    mutex_unlock(&buffer_lock);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
};

//=============================================================================
//  Настройка sysfs
// Функция получения размера буфера из sysfs
static ssize_t size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%zu\n", buffer_size);
}

// Функция изменения размера буфера
static ssize_t size_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    size_t new_size;
    char *new_buffer;

    if (kstrtoul(buf, 10, &new_size) || new_size == 0)
        return -EINVAL;

    if (mutex_lock_interruptible(&buffer_lock))
        return -ERESTARTSYS;

    new_buffer = krealloc(ring_buffer, new_size, GFP_KERNEL);
    if (!new_buffer)
    {
        mutex_unlock(&buffer_lock);
        return -ENOMEM;
    }

    ring_buffer = new_buffer;
    buffer_size = new_size;
    // Сбрасываем состояние, так как старые индексы могут выйти за границы нового размера
    head = 0;
    tail = 0;
    is_full = false;

    mutex_unlock(&buffer_lock);
    return count;
}

// Создаем атрибут: имя файла, права (чтение/запись), функции show/store
static struct kobj_attribute size_attr = __ATTR(buf_size, 0664, size_show, size_store);
static struct kobject *ring_kobj;

//=============================================================================
// Инициализация и выгрузка модуля
static int __init buffer_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    ring_buffer = kmalloc(buffer_size, GFP_KERNEL);
    if (!ring_buffer)
    {
        ret = -ENOMEM;
        goto err_num;
    }

    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0)
        goto err_mem;

    buffer_class = class_create(CLASS_NAME);
    if (IS_ERR(buffer_class))
    {
        ret = PTR_ERR(buffer_class);
        goto err_cdev;
    }

    buffer_device = device_create(buffer_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(buffer_device))
    {
        ret = PTR_ERR(buffer_device);
        goto err_class;
    }

    ring_kobj = kobject_create_and_add("ring_buffer_kobj", kernel_kobj);
    if (!ring_kobj)
    {
        ret = -ENOMEM;
        goto err_dev;
    }

    ret = sysfs_create_file(ring_kobj, &size_attr.attr);
    if (ret)
        goto err_kobj;

    pr_info("RingBuffer: Module loaded. Major: %d, Device: /dev/%s\n", MAJOR(dev_num), DEVICE_NAME);
    return 0;

// Каскадная очистка при ошибках
err_kobj:
    kobject_put(ring_kobj);
err_dev:
    device_destroy(buffer_class, dev_num);
err_class:
    class_destroy(buffer_class);
err_cdev:
    cdev_del(&my_cdev);
err_mem:
    kfree(ring_buffer);
err_num:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit buffer_exit(void)
{
    // Удаляем все в обратном порядке
    sysfs_remove_file(ring_kobj, &size_attr.attr);
    kobject_put(ring_kobj);

    device_destroy(buffer_class, dev_num);
    class_destroy(buffer_class);

    cdev_del(&my_cdev);
    kfree(ring_buffer);
    unregister_chrdev_region(dev_num, 1);

    pr_info("RingBuffer: Module unloaded\n");
}

module_init(buffer_init);
module_exit(buffer_exit);