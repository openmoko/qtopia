/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "docparatable.h"

struct entry_table
{
    const char *name;
    const char *help;
    int (*read_func)(int fd);
    int (*write_func)(int fd, const char *value);
};

int read_bdaddr(int fd)
{
    char bdaddr[6];
    if (ioctl(fd, DOCPTGETBTMACADDR, bdaddr) < 0) {
        return 1;
    }

    fprintf(stdout, "%02X:%02X:%02X:%02X:%02X:%02X\n", bdaddr[0], bdaddr[1], bdaddr[2],
            bdaddr[3], bdaddr[4], bdaddr[5]);

    return 0;
}

int write_bdaddr(int fd, const char *value)
{
    char bdaddr[6];
    unsigned int tmp[6];
    int i;

    sscanf(value, "%02x:%02x:%02x:%02x:%02x:%02x", &tmp[0], &tmp[1], &tmp[2],
           &tmp[3], &tmp[4], &tmp[5]);

    for (i = 0; i < 6; i++) {
        bdaddr[i] = tmp[i];
    }

    if (ioctl(fd, DOCPTSETBTMACADDR, bdaddr) < 0) {
        return 1;
    }

    return 0;
}

struct entry_table entries[] = { {"bdaddr", "Get/Set Bluetooth Address", read_bdaddr, write_bdaddr},
                                 {0, 0, 0, 0}
                               };

void print_help(const char *s)
{
    struct entry_table *entry;

    fprintf(stderr, "Useage: %s entry [value]\n", s);
    fprintf(stderr, "Read / Write a value into the TAT table\n");
    fprintf(stderr, "The possible keys are:\n");

    entry = entries;
    while (entry->name != NULL) {
        fprintf(stderr, "%s - %s\n", entry->name, entry->help);
        entry++;
    }
}

int main(int argc, char *argv[])
{
    int fd = 0;
    struct entry_table *entry;
    int status;

    if (argc < 2 || argc > 3) {
        print_help(argv[0]);
        return 1;
    }

    fd = open("/dev/docparatable", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Could not open the TAT table!\n");
        return 2;
    }

    entry = entries;
    while (entry->name != NULL) {
        if (!strcmp(entry->name, argv[1])) {
            break;
        }
        entry++;
    }

    if (entry->name == NULL) {
        fprintf(stderr, "Don't know how to read/write entry %s\n", argv[1]);
        return 3;
    }

    if (argc == 3) {
        status = entry->write_func(fd, argv[2]);
    }
    else {
        status = entry->read_func(fd);
    }

    if (status != 0) {
        fprintf(stderr, "Trouble reading/writing entry %s\n", argv[1]);
        return 4;
    }

    close(fd);

    return 0;
}

