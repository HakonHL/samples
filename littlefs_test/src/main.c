/*
 * Copyright (c) 2022 Lukasz Majewski, DENX Software Engineering GmbH
 * Copyright (c) 2019 Peter Bigot Consulting, LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Sample which uses the filesystem API with littlefs */

#include <stdio.h>

#include <zephyr.h>
#include <device.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <logging/log.h>
#include <storage/flash_map.h>
#include <errno.h>

LOG_MODULE_REGISTER(main);

struct fs_file_t file;

uint8_t data[] = "Wake up \n \
(Wake up) \n \
Grab a brush and put a little make-up \n \
Hide the scars to fade away the shake-up \n \
(Hide the scars to fade away the...) \n \
Why'd you leave the keys upon the table? \n \
Here you go create another fable \n \
You wanted to \n \
Grab a brush and put a little makeup \n \
You wanted to \n \
Hide the scars to fade away the shake-up \n \
You wanted to \n \
Why'd you leave the keys upon the table? \n \
You wanted to \n \
I don't think you trust \n \
In... My... Self-righteous suicide \n \
I... Cry... When angels deserve to die \n \
Wake up \n \
(Wake up) \n \
Grab a brush and put a little make-up \n \
Hide the scars to fade away the \n \
(Hide the scars to fade away the shake-up) \n \
Why'd you leave the keys upon the table? \n \
Here you go create another fable \n \
You wanted to \n \
Grab a brush and put a little make-up \n \
You wanted to \n \
Hide the scars to fade away the shake-up \n \
You wanted to \n \
Why'd you leave the keys upon the table? \n \
You wanted to \n \
I don't think you trust \n \
In... My... Self-righteous suicide \n \
I... Cry... When angels deserve to die \n \
In, my, Self-righteous suicide \n \
I, cry, when angels deserve to die \n \
Father (father) \n \
Father (father) \n \
Father (father) \n \
Father (father) \n \
Father, into your hands I commend my spirit \n \
Father, into your hands \n \
Why have you forsaken me? \n \
In your eyes forsaken me \n \
In your thoughts forsaken me \n \
In your heart forsaken me, oh \n \
Trust in my self-righteous suicide \n \
I cry when angels deserve to die \n \
In my self-righteous suicide \n \
I cry when angels deserve to die";
uint8_t data2[255];

int write_stuff(struct fs_file_t *file, void *data, int size)
{
	int ret;
	ret = fs_seek(file, 0, FS_SEEK_SET);
	if (ret<0) {
		LOG_ERR("seek fail!");
	}
	ret = fs_write(file, data, size);
	if (ret<0) {
		LOG_ERR("write fail!");
	}
	LOG_INF("write %d bytes", ret);
	fs_sync(file);
	return ret;
}

int read_stuff(struct fs_file_t *file, void *data, int size)
{
	int ret, c = 0;
	ret = fs_seek(file, 0, FS_SEEK_SET);
	if (ret<0) {
		LOG_ERR("seek fail!");
	}
	do {
		ret = fs_read(file, data, size);
		LOG_INF("read; %s", (char*) data);
		c+=ret;
	} while (ret > 0);
	if (ret<0) {
		LOG_ERR("read fail!");
		return ret;
	}

	LOG_INF("read %d bytes", c);
	return c;
}

void main(void)
{
	int ret;
	fs_file_t_init(&file);

	ret = fs_open(&file, "/lfs1/test", FS_O_CREATE | FS_O_RDWR);
	if (ret<0) {
		LOG_ERR("open fail!");
	}
	write_stuff(&file, data, sizeof(data));
	read_stuff(&file, data2, sizeof(data2));
}
