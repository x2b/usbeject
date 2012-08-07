#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int usb_unmount_dev(const char *devname, struct udev *udev)
{
  struct udev_enumerate *enumerate;
  struct udev_list_entry *dev_list, *dev_list_entry;
  struct udev_device *device;
  int device_found = 0;
  int retval = 0;
  FILE *remfile = NULL;
  
  
  enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "block");
  udev_enumerate_scan_devices(enumerate);
  dev_list = udev_enumerate_get_list_entry(enumerate);

  udev_list_entry_foreach(dev_list_entry, dev_list)
  {
    const char *path;

    path = udev_list_entry_get_name(dev_list_entry);
    device = udev_device_new_from_syspath(udev, path);
    if(!strcmp(udev_device_get_devnode(device), devname))
    {
      device_found = 1;
      break;
    }
  }
  if(!device_found)
  {
    fprintf(stderr, "Could not find block device with path %s\n",
	   devname);
    goto cleanup;
  }

  // now, the device is set to the device that we need

  device = udev_device_get_parent_with_subsystem_devtype(
    device,
    "usb",
    "usb_device");
  if(!device)
  {
    fprintf(stderr, "Could not find parent USB device\n");
    goto cleanup;
  }

  if(chdir(udev_device_get_syspath(device)))
  {
    fprintf(stderr, "Could not chdir to the correct syspath\n");
    goto cleanup;
  }

  remfile = fopen("remove", "w");
  if(!remfile)
  {
    perror("Could not open `remove` file");
    goto cleanup;
  }

  if(EOF == fputs("1", remfile))
  {
    fprintf(stderr, "Could not write to `remove` file\n");
    goto cleanup;
  }
  
  retval = 1;

cleanup:
  udev_enumerate_unref(enumerate);
  if(remfile)
    fclose(remfile);
  return retval;
}

int main (int argc, char **argv)
{
  struct udev *udev;
  const char *devname;

  if(argc <= 1)
  {
    fprintf(stderr, "Usage: %s <devname>\n", argv[0]);
    return 1;
  }

  devname = argv[1];

  /* Create the udev object */
  udev = udev_new();
  if (!udev) {
    fprintf(stderr, "Could not create udev\n");
    return 1;
  }

  if(!usb_unmount_dev(devname, udev))
  {
    fprintf(stderr, "Could not eject device %s\n", devname);
    udev_unref(udev);
    return 1;
  }

  udev_unref(udev);
  return 0;
}
