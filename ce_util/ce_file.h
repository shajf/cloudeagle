/*
 * =====================================================================================
 *
 *       Filename:  ce_file.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */

#ifndef _CE_FILE_H
#define _CE_FILE_H

#include "ce_basicdefs.h"
#include "ce_string.h"
#include "ce_errno.h"
#include "ce_times.h"
#include "ce_time.h"
#include "ce_palloc.h"
#include "ce_atomic.h"

typedef int		ce_fd_t;
typedef struct stat 	ce_file_info_t;
typedef ino_t		ce_file_uniq_t;

#define CE_INVALID_FILE	-1
#define CE_FILE_ERROR		-1

typedef struct {
	u_char* name;
	size_t size;
	void *addr;
	ce_fd_t fd;
	//ce_log_t *log;
}ce_file_mapping_t;

#define CE_FILE_MAPPING_T_SIZE sizeof(ce_file_mapping_t)

typedef struct {
	DIR *dir;
	struct dirent *de;
	struct stat info;
	unsigned type:8;
	unsigned valid_info:1;
}ce_dir_t;

#define CE_DIR_T_SIZE sizeof(ce_dir_t)

typedef struct {
	size_t n;
	glob_t pglob;
	u_char *pattern;
	//ce_log_t *log
	ce_uint_t test;
}ce_glob_t;

#define CE_GLOB_T_SIZE sizeof(ce_glob_t)

typedef struct{
	ce_fd_t  fd;
	ce_str_t  name;
	ce_file_info_t info;

	off_t offset;
	off_t sys_offset;

//  ce_log_t *log;

	unsigned valid_info:1;
}ce_file_t;

#define CE_FILE_T_SIZE sizeof(ce_file_t)

typedef struct {
	ce_fd_t fd;
	ce_str_t  name;

	u_char *buffer;
	u_char *pos;
	u_char *last;
}ce_open_file_t;

#define CE_MAX_PATH_LEVEL  3

typedef time_t (*ce_path_manager_pt) (void *data);
typedef void (*ce_path_loader_pt) (void *data);

typedef struct {
	ce_str_t name;
	size_t len;
	size_t level[3];
	ce_path_manager_pt  manager;
	ce_path_loader_pt loader;
	void *data;
	u_char *conf_file;
	ce_uint_t line;
} ce_path_t;

#define CE_PATH_T_SIZE sizeof(ce_path_t)

typedef struct {
	ce_str_t name;
	size_t level[3];
} ce_path_init_t;

#define CE_PATH_INIT_T sizeof(ce_path_init_t)

typedef struct 
{
	ce_file_t  file;
	off_t offset;
	ce_path_t *path;
	ce_pool_t *pool;
	char *warn;
	ce_uint_t  access;
	unsigned  log_level:8;
	unsigned  persistent:1;
	unsigned  clean:1;
} ce_temp_file_t;

#define CE_TEMP_FILE_T_SIZE sizeof(ce_temp_file_t)

typedef struct {
	ce_uint_t access;
	ce_uint_t path_access;
	time_t time;
	ce_fd_t fd;

	unsigned create_path:1;
	unsigned delete_file:1;

//    ce_log_t                 *log;
} ce_ext_rename_file_t;

#define CE_EXT_RENAME_FILE_T_SIZE sizeof(ce_ext_rename_file_t)

typedef struct {
	off_t size;
	size_t buf_size;

	ce_uint_t access;
	time_t  time;

  //  ce_log_t                 *log;
} ce_copy_file_t;

#define CE_COPY_FILE_T_SIZE sizeof(ce_copy_file_t)
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern ce_int_t ce_create_temp_file(ce_file_t *file, ce_path_t *path,
				     	ce_pool_t *pool, ce_uint_t persistent, 
					ce_uint_t clean,ce_uint_t access);

extern ce_int_t ce_create_path(ce_file_t *file, ce_path_t *path);

extern ce_err_t ce_create_full_path(u_char *dir, ce_uint_t access);

extern ce_int_t ce_ext_rename_file(ce_str_t *src, ce_str_t *to,
				       ce_ext_rename_file_t *ext);

extern ce_int_t ce_copy_file(u_char *from, u_char *to, ce_copy_file_t *cf);

extern ce_atomic_uint_t ce_next_temp_number(ce_uint_t collision);

extern ce_fd_t ce_open_tempfile(u_char *name,
			           ce_uint_t persistent,
				    ce_uint_t access);
#define ce_open_tempfile_name   "open()"

extern ssize_t ce_read_file(ce_file_t *file,u_char *buf,size_t size,
			      off_t offset);
#define ce_read_file_name	"pread"

extern ssize_t ce_file_getc(ce_file_t *file,char *ch);

extern ssize_t ce_file_gets(ce_file_t *thefile,char* str,size_t len);

extern ce_int_t ce_filepath_root(const char **rootpath,const char **inpath,int32_t flags,ce_pool_t *p);

extern ssize_t ce_write_file(ce_file_t *file,u_char* buf,size_t size,
			       off_t offset);
extern ce_int_t ce_set_file_time(u_char *name,ce_fd_t fd,time_t s);

#define ce_set_file_time_name	"utimes()"


extern void  ce_create_hashed_filename(ce_path_t *path,
				         u_char* name, size_t len);

extern ce_int_t ce_create_file_mapping(ce_file_mapping_t *fm);

extern void ce_close_file_mapping(ce_file_mapping_t *fm);

extern ce_int_t ce_open_dir(ce_str_t* name,ce_dir_t *dir);
#define ce_open_dir_name   "opendir()"
extern size_t ce_fs_bsize(u_char *name);

extern ce_int_t ce_read_dir(ce_dir_t *dir);
#define ce_read_dir_name           "readdir()"

extern ce_int_t ce_open_glob(ce_glob_t *gl);
#define ce_open_glob_name          "glob()"

extern ce_int_t ce_read_glob(ce_glob_t *gl, ce_str_t *name);
extern void ce_close_glob(ce_glob_t *gl);


extern ce_err_t ce_trylock_fd(ce_fd_t fd);
extern ce_err_t ce_lock_fd(ce_fd_t fd);
extern ce_err_t ce_unlock_fd(ce_fd_t fd);

#define ce_trylock_fd_name         "fcntl(F_SETLK, F_WRLCK)"
#define ce_lock_fd_name            "fcntl(F_SETLKW, F_WRLCK)"
#define ce_unlock_fd_name          "fcntl(F_SETLK, F_UNLCK)"


extern ce_atomic_t      *ce_temp_number;

extern ce_atomic_int_t   ce_random_number;

///////////////////////////////////////////////////////////////////////////////////////
/*common file op micro fun defines*/
///////////////////////////////////////////////////////////////////////////////////////
#define CE_FILE_RDONLY	O_RDONLY
#define CE_FILE_WRONLY	O_WRONLY
#define CE_FILE_RDWR		O_RDWR
#define CE_FILE_CREATE_OR_OPEN O_CREAT
#define CE_FILE_OPEN		0
#define CE_FILE_TRUNCATE	O_CREAT|O_TRUNC
#define CE_FILE_APPEND	O_WRONLY|O_APPEND
#define CE_FILE_NONBLOCK 	O_NONBLOCK

#define CE_FILE_DEFAULT_ACCESS	0644
#define CE_FILE_OWNER_ACCESS		0600

#define ce_open_file(name,mode,create,access) \
	open((const char*)name,mode|create,access)
#define ce_open_file_name	"open()"

#define ce_close_file	close
#define ce_close_file_name	"close()"

#define ce_delete_file(name)	unlink((const char*)name)
#define ce_delete_file_name	"unlink()"

#define ce_read_fd	read
#define ce_read_fd_name  "read()"

static inline ssize_t 
ce_write_fd(ce_fd_t fd,void *buf,size_t n)
{
	return write(fd,buf,n);
}

static inline int 
ce_file_is_existed(u_char* name)
{
	ce_fd_t fd=ce_open_file(name,
				    O_RDONLY,
				    CE_FILE_OPEN,
				    CE_FILE_DEFAULT_ACCESS);
	ce_close_file(fd);
	return (fd>=0);
}

static inline int
ce_dir_is_existed(u_char *dir_name)
{
	ce_str_t dir_str;
	ce_dir_t dir;
	dir_str.data=dir_name;
	dir_str.len=ce_strlen(dir_name);

	int rc=ce_open_dir(&dir_str,&dir);

	return rc==CE_OK;
}

static inline int
ce_get_file_size(const u_char *name)
{
	struct stat sb;
	if(stat((const char*)name,&sb)!=0)
	{
		return 0;
	}

	return sb.st_size;
}

#define ce_write_fd_name   "write()"

#define ce_write_console	ce_write_fd

#define ce_linefeed(p)	*p++=LF
#define CE_LINEFEED_SIZE	1
#define CE_LINEFEED		"\x0a"

#define ce_rename_file(o,n)	rename((const char*)o,(const char*)n)
#define ce_rename_file_name	"rename()"

#define ce_change_file_access(n,a) 	chmod((const char*)n,a)
#define ce_change_file_access_name	"chmod()"

#define ce_file_info(file,sb)	stat((const char*) file,sb)
#define ce_file_info_name	"stat()"

#define ce_fd_info(fd,sb)	fstat(fd,sb)
#define ce_fd_info_name	"fstat()"

#define ce_link_info(file,sb)	lstat((const char*)file,sb)
#define	ce_link_info_name	"lstat()"

#define ce_is_dir(sb)		(S_ISDIR((sb)->st_mode))
#define ce_is_file(sb)	(S_ISREG((sb)->st_mode))
#define ce_is_link(sb)	(S_ISLNK((sb)->st_mode))
#define ce_is_exec(sb)	(((sb)->st_mode&S_IXUSR)==S_IXUSR)

#define ce_file_access(sb)	((sb)->st_mode&0777)
#define ce_file_size(sb)      (sb)->st_size
#define ce_file_fs_size(sb)   ((sb)->st_blocce*512)
#define ce_file_mtime(sb)	(sb)->st_mtime
#define ce_file_unig(sb)	(sb)->st_ino

#define ce_filename_cmp	ce_memcmp

#define ce_realpath(p,r)	realpath((char*)p,(char*)r)
#define ce_realpath_name	"realpath()"

#define ce_getcwd(buf,size)	getcwd((char*)buf,size)
#define ce_getcwd_name	"getcwd()"

#define ce_path_separator(c)	((c)=='/')

#define CE_MAX_PATH	PATH_MAX
#define CE_DIR_MASK_LEN	0

#define ce_close_dir(d)	closedir((d)->dir)
#define ce_close_dir_name	"closedir()"

#define ce_create_dir(name, access) mkdir((const char *) name, access)
#define ce_create_dir_name         "mkdir()"

#define ce_delete_dir(name)     rmdir((const char *) name)
#define ce_delete_dir_name         "rmdir()"

#define ce_dir_access(a)        (a | (a & 0444) >> 2)

#define ce_de_name(dir)         ((u_char *) (dir)->de->d_name)

#define ce_de_namelen(dir)      ce_strlen((dir)->de->d_name)

static inline ce_int_t
ce_de_info(u_char *name, ce_dir_t *dir)
{
    dir->type = 0;
    return stat((const char *) name, &dir->info);
}

#define ce_de_info_n            "stat()"

#define ce_de_link_info(name, dir)  lstat((const char *) name, &(dir)->info)
#define ce_de_link_info_n       "lstat()"

#define ce_de_is_dir(dir)       (S_ISDIR((dir)->info.st_mode))
#define ce_de_is_file(dir)      (S_ISREG((dir)->info.st_mode))
#define ce_de_is_link(dir)      (S_ISLNK((dir)->info.st_mode))

#define ce_de_access(dir)       (((dir)->info.st_mode) & 0777)
#define ce_de_size(dir)         (dir)->info.st_size
#define ce_de_fs_size(dir)      ((dir)->info.st_blocce * 512)
#define ce_de_mtime(dir)        (dir)->info.st_mtime

#define ce_stderr               STDERR_FILENO
#define ce_set_stderr(fd)       dup2(fd, STDERR_FILENO)
#define ce_set_stderr_n         "dup2(STDERR_FILENO)"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _CE_FILE_H */
