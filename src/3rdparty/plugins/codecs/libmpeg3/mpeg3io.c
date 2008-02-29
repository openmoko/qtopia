#include "mpeg3private.h"
#include "mpeg3protos.h"

#ifndef _WIN32
#include <mntent.h>
#else

#endif
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

mpeg3_fs_t* mpeg3_new_fs(char *path)
{
	mpeg3_fs_t *fs = (mpeg3_fs_t*)calloc(1, sizeof(mpeg3_fs_t));
	fs->css = mpeg3_new_css();
	strcpy(fs->path, path);
	return fs;
}

int mpeg3_delete_fs(mpeg3_fs_t *fs)
{
	mpeg3_delete_css(fs->css);
	free(fs);
	return 0;
}

int mpeg3_copy_fs(mpeg3_fs_t *dst, mpeg3_fs_t *src)
{
	strcpy(dst->path, src->path);
	dst->current_byte = 0;
	return 0;
}

long mpeg3io_get_total_bytes(mpeg3_fs_t *fs)
{
/*
 * 	struct stat st;
 * 	if(stat(fs->path, &st) < 0) return 0;
 * 	return (long)st.st_size;
 */

	fseek(fs->fd, 0, SEEK_END);
	fs->total_bytes = ftell(fs->fd);
	fseek(fs->fd, 0, SEEK_SET);
	if ( fs->total_bytes >= 128 && fs->has_id3_tag )
	    fs->total_bytes -= 128;
	return fs->total_bytes;
}

int mpeg3io_open_file(mpeg3_fs_t *fs)
{
/* Need to perform authentication before reading a single byte. */
	mpeg3_get_keys(fs->css, fs->path);

	if(!(fs->fd = fopen(fs->path, "rb")))
	{
		perror("mpeg3io_open_file");
		return 1;
	}

	// Set the id3 tag flag
	fs->has_id3_tag = 0;
	char id3v1[3];
	// seek to the position in the file where an ID tag is expected
	if ( fseek( fs->fd, -128, SEEK_END ) == 0 )
	    // read in the id3 tag data
	    if ( fread( id3v1, 1, 3, fs->fd ) == 3 )
		// check the tag header
		if ( ::strncmp( (const char *)id3v1, "TAG", 3 ) == 0 )
		    fs->has_id3_tag = 1;

	fs->total_bytes = mpeg3io_get_total_bytes(fs);
	
	if(!fs->total_bytes)
	{
		fclose(fs->fd);
		return 1;
	}
	fs->current_byte = 0;
	return 0;
}

int mpeg3io_close_file(mpeg3_fs_t *fs)
{
	if(fs->fd) fclose(fs->fd);
	fs->fd = 0;
	return 0;
}

int mpeg3io_read_data(unsigned char *buffer, long bytes, mpeg3_fs_t *fs)
{
	int result = 0;
//printf("read %d bytes\n",bytes);
	result = !fread(buffer, 1, bytes, fs->fd);
	
	if ( fs->current_byte >= fs->total_bytes )
	    return 0;

	fs->current_byte += bytes;
	return (result && bytes);
}

int mpeg3io_device(char *path, char *device)
{
	struct stat file_st, device_st;
    struct mntent *mnt;
	FILE *fp;

	if(stat(path, &file_st) < 0)
	{
		perror("mpeg3io_device");
		return 1;
	}

#ifndef _WIN32
	fp = setmntent(MOUNTED, "r");
    while(fp && (mnt = getmntent(fp)))
	{
		if(stat(mnt->mnt_fsname, &device_st) < 0) continue;
		if(device_st.st_rdev == file_st.st_dev)
		{
			strncpy(device, mnt->mnt_fsname, MPEG3_STRLEN);
			break;
		}
	}
	endmntent(fp);
#endif

	return 0;
}

int mpeg3io_seek(mpeg3_fs_t *fs, long byte)
{
	fs->current_byte = byte;
	return fseek(fs->fd, byte, SEEK_SET);
}

int mpeg3io_seek_relative(mpeg3_fs_t *fs, long bytes)
{
	fs->current_byte += bytes;
	return fseek(fs->fd, fs->current_byte, SEEK_SET);
}

