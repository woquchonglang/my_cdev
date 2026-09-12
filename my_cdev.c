#include "./my_cdev.h"
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#define MAJOR_NUM 388

struct my_cdev {

  int len;
  unsigned char buffer[50];
  struct cdev cdev;
};

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("my_cdev");

static dev_t dev_num = {0};

struct my_cdev *gcd;

struct class *cls;

static int ndevices = 1;

module_param(ndevices, int, 0644);
MODULE_PARM_DESC(ndevices, "the number of devices for register.\n");

static int my_cdev_open(struct inode *inode, struct file *file) {
  struct my_cdev *dev;
  printk("my_cdev: open success!\n");
  dev = container_of(inode->i_cdev, struct my_cdev, cdev);
  file->private_data = dev;
  return 0;
}

static ssize_t my_cdev_read(struct file *file, char __user *ubuf, size_t size,
                            loff_t *ppos) {
  int n;
  int ret;
  char *kbuf;
  struct my_cdev *my_dev = file->private_data;
  printk("my_cdev: read *ppos: %lld\n", *ppos);
  if (*ppos == my_dev->len) {
    return 0;
  }
  if (size > my_dev->len - *ppos) {
    n = my_dev->len - *ppos;
  } else {
    n = size;
  }
  printk("my_cdev: read n = %d\n", n);
  kbuf = my_dev->buffer + *ppos;
  ret = copy_to_user(ubuf, kbuf, n);
  if (ret != 0) {
    return -EFAULT;
  }
  *ppos += n;
  printk("my_cdev: read success!\n");

  return n;
}

static ssize_t my_cdev_write(struct file *file, const char *__user ubuf,
                             size_t size, loff_t *ppos) {

  int n;
  int ret;
  char *kbuf;
  struct my_cdev *my_dev = file->private_data;
  printk("my_cdev: write *ppos: %lld\n", *ppos);
  if (*ppos == sizeof(my_dev->buffer)) {
    return -1;
  } else {
    n = size;
  }
  kbuf = my_dev->buffer + *ppos;
  ret = copy_from_user(kbuf, ubuf, n);
  if (ret != 0)
    return -EFAULT;
  *ppos += n;
  my_dev->len += n;
  printk("my_cdev: write success!\n");
  return n;
}

static long my_cdev_unlocked_ioctl(struct file *file, unsigned int cmd,
                                   unsigned long arg) {
  int ret = 0;
  struct my_cdev *my_dev = file->private_data;

  if (_IOC_TYPE(cmd) != MY_CDEV_TYPE) {
    pr_err("cmd %u,bad magic 0x%x/0x%x.\n", cmd, _IOC_TYPE(cmd), MY_CDEV_TYPE);
    return -ENOTTY;
  } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
    ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
  }
  if (ret) {
    pr_err("bad access %d.\n", ret);
    return -EFAULT;
  }
  switch (cmd) {
  case MY_CDEV_CLEAN:
    printk("my_cdev: cmd: clean\n");
    memset(my_dev->buffer, 0, sizeof(my_dev->buffer));
    break;
  case MY_CDEV_SETVALUE:
    printk("my_cdev: cmd: setvalue\n");
    my_dev->len = arg;
    break;
  case MY_CDEV_GETVALUE:

    printk("my_cdev: cmd: getvalue\n");
    ret = put_user(my_dev->len, (int *)arg);
    break;
  default:
    return -EFAULT;
  }
  return ret;
}

static const struct file_operations my_cdev_operations = {
    .owner = THIS_MODULE,
    .open = my_cdev_open,
    .read = my_cdev_read,
    .write = my_cdev_write,
    .unlocked_ioctl = my_cdev_unlocked_ioctl};

static int __init my_cdev_init(void) {
  int i = 0;
  int n = 0;
  int ret;
  gcd = kzalloc(ndevices * sizeof(struct my_cdev), GFP_KERNEL);
  if (!gcd) {
    return -ENOMEM;
  }

  dev_num = MKDEV(MAJOR_NUM, 0);
  ret = register_chrdev_region(dev_num, ndevices, "my_cdev");
  if (ret < 0) {
    printk("my_cdev: fail to register_chrdev_region\n");
    goto err_kfree;
  }

  cls = class_create("my_cdev");
  if (IS_ERR(cls)) {
    ret = PTR_ERR(cls);
    goto err_unreg;
  }

  printk("my_cdev: ndevices: %d\n", ndevices);
  for (n = 0; n < ndevices; n++) {
    cdev_init(&gcd[n].cdev, &my_cdev_operations);
    ret = cdev_add(&gcd[n].cdev, dev_num + n, 1);
    if (ret < 0) {
      goto err_cdev_add;
    }
    struct device *device =
        device_create(cls, NULL, dev_num + n, NULL, "my_dev%d", n);
    if (IS_ERR(device)) {
      ret = PTR_ERR(device);
      printk("my_cdev: fail to device_create\n");
      goto err_device_create;
    }
  }

  printk("my_cdev: register my_cdev to system,ok!\n");
  return 0;

err_device_create:
  for (i = 0; i < n; i++) {
    device_destroy(cls, dev_num + i);
  }

err_cdev_add:
  for (i = 0; i < n; i++) {
    cdev_del(&gcd[i].cdev);
  }
  class_destroy(cls);

err_unreg:
  unregister_chrdev_region(dev_num, ndevices);

err_kfree:
  kfree(gcd);
  gcd = NULL;
  return ret;
}

static void __exit my_cdev_exit(void) {
  int i;
  for (i = 0; i < ndevices; i++) {
    device_destroy(cls, dev_num + i);
  }
  for (i = 0; i < ndevices; i++) {
    cdev_del(&gcd[i].cdev);
  }
  class_destroy(cls);
  unregister_chrdev_region(dev_num, ndevices);
  kfree(gcd);
  gcd = NULL;
  printk("my_cdev: unload ok\n");
  return;
}

module_init(my_cdev_init);
module_exit(my_cdev_exit);
