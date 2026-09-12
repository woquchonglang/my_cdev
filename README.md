## my linux cdev

A simple character device driver

### requirements

arch linux:
```sh
sudo pacman -S linux-zen-headers
```

### get started

build module
```sh
bear -- make
```

load module

```sh
sudo insmod my_cdev.ko
```

create my dev (when no udev/mdev)

```sh
sudo mknod /dev/my_dev0 c 388 0
```

test the dev

```sh
gcc app.c -o app
sudo ./app
```

rm modlue
```sh
sudo rmmod my_cdev
```

rm my_cdev (when no udev/mdev)
```sh
sudo rm my_cdev
```

### debug
```sh
sudo dmesg | grep "my_cdev:*"
```
