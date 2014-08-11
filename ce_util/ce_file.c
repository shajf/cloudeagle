/*
 * =====================================================================================
 *
 *       Filename:  ce_file.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  CE,2013,CE TEAM
 *
 * =====================================================================================
 */

#include "ce_file.h"
#include "ce_alloc.h"

static ce_atomic_t   temp_number = 0;
ce_atomic_t         *ce_temp_number = &temp_number;
ce_atomic_int_t      ce_random_number = 123456;

ce_int_t ce_create_temp_file(ce_file_t *file,
				 ce_path_t *path,
			         ce_pool_t *pool,
				 ce_uint_t persistent, 
                                 ce_uint_t clean, 
				 ce_uint_t m_access)
{
    clean=clean;
    uint32_t                  n;
    ce_err_t                 err;
    //ce_pool_cleanup_t       *cln;
    //ce_pool_cleanup_file_t  *clnf;

    file->name.len = path->name.len + 1 + path->len + 10;

    file->name.data = (u_char*)ce_pnalloc(pool, file->name.len + 1);
    if (file->name.data == NULL) {
        return CE_ERROR;
    }

    ce_memcpy(file->name.data, path->name.data, path->name.len);
   
 
   n = (uint32_t) ce_next_temp_number(0);
    #if 0
    cln = ce_pool_cleanup_add(pool, sizeof(ce_pool_cleanup_file_t));
    if (cln == NULL) {
        return CE_ERROR;
    }
    #endif

    for ( ;; ) {
        
	
	(void) ce_sprintf(file->name.data + path->name.len + 1 + path->len,
                           "%010uD%Z", n);

        ce_create_hashed_filename(path, file->name.data, file->name.len);

	#if 0
       	ce_log_debug1(CE_LOG_DEBUG_CORE, file->log, 0,
                       "hashed path: %s", file->name.data);
	#endif

        file->fd = ce_open_tempfile(file->name.data, persistent,m_access);
        #if 0
        ce_log_debug1(CE_LOG_DEBUG_CORE, file->log, 0,
                       "temp fd:%d", file->fd);
	#endif

        if (file->fd != CE_INVALID_FILE) {
	
	   #if 0
            cln->handler = clean ? ce_pool_delete_file : ce_pool_cleanup_file;
            clnf = cln->data;

            clnf->fd = file->fd;
            clnf->name = file->name.data;
            clnf->log = pool->log;
            #endif 

            return CE_OK;
        }

        err = ce_errno;

        if (err == CE_EEXIST) {
            n = (uint32_t) ce_next_temp_number(1);
            continue;
        }

        if ((path->level[0] == 0) || (err != CE_ENOPATH)) {
            //ce_log_error(CE_LOG_CRIT, file->log, err,
              //            ce_open_tempfile_n " \"%s\" failed",
                //          file->name.data);
            return CE_ERROR;
        }

        if (ce_create_path(file, path) == CE_ERROR) {
            return CE_ERROR;
        }
    }
}


void
ce_create_hashed_filename(ce_path_t *path, u_char *file, size_t len)
{
    size_t      i, level;
    ce_uint_t  n;

    i = path->name.len + 1;

    file[path->name.len + path->len]  = '/';

    for (n = 0; n < 3; n++) {
        level = path->level[n];

        if (level == 0) {
            break;
        }

        len -= level;
        file[i - 1] = '/';
        ce_memcpy(&file[i], &file[len], level);
        i += level + 1;
    }
}


ce_int_t ce_create_path(ce_file_t *file, ce_path_t *path)
{
    size_t      pos;
    ce_err_t   err;
    ce_uint_t  i;

    pos = path->name.len;

    for (i = 0; i < 3; i++) {
        if (path->level[i] == 0) {
            break;
        }

        pos += path->level[i] + 1;

        file->name.data[pos] = '\0';
        #if 0
        ce_log_debug1(CE_LOG_DEBUG_CORE, file->log, 0,
                       "temp file: \"%s\"", file->name.data);
	#endif
        if (ce_create_dir(file->name.data, 0700) == CE_FILE_ERROR) {
            err = ce_errno;
            if (err != CE_EEXIST) {
		    #if 0
		    ce_log_error(CE_LOG_CRIT, file->log, err,
                              ce_create_dir_n " \"%s\" failed",
                              file->name.data);
		     #endif
                return CE_ERROR;
            }
        }

        file->name.data[pos] = '/';
    }

    return CE_OK;
}


ce_err_t ce_create_full_path(u_char *dir, ce_uint_t m_access)
{
    u_char     *p, ch;
    ce_err_t   err;
    err = 0;
    p = dir + 1;
    for ( /* void */ ; *p; p++) {
        ch = *p;

        if (ch != '/') {
            continue;
        }

        *p = '\0';

        if (ce_create_dir(dir, m_access) == CE_FILE_ERROR) {
            err = ce_errno;

            switch (err) {
            case CE_EEXIST:
                err = 0;
            case CE_EACCES:
                break;

            default:
                return err;
            }
        }

        *p = '/';
    }

    return err;
}


ce_atomic_uint_t ce_next_temp_number(ce_uint_t collision)
{
    ce_atomic_uint_t  n, add;

    add = collision ? ce_random_number : 1;

    n = ce_atomic_add(ce_temp_number, add);

    return n + add;
}

ce_int_t ce_ext_rename_file(ce_str_t *src, ce_str_t *to, 
				ce_ext_rename_file_t *ext)
{
    u_char           *name;
    ce_err_t         m_err;
    ce_copy_file_t   cf;

    if (ext->access) {
        if (ce_change_file_access(src->data, ext->access) == CE_FILE_ERROR) {
#if 0
	    ce_log_error(CE_LOG_CRIT, ext->log, ce_errno,
                          ce_change_file_access_n " \"%s\" failed", src->data);
#endif
            m_err = 0;
            goto failed;
        }
    }


    if (ext->time != -1) {
        if (ce_set_file_time(src->data, ext->fd, ext->time) != CE_OK) {
#if 0
		ce_log_error(CE_LOG_CRIT, ext->log, ce_errno,
	     		    ce_set_file_time_n " \"%s\" failed", src->data);
#endif
            m_err = 0;
            goto failed;
        }
    }

    if (ce_rename_file(src->data, to->data) != CE_FILE_ERROR) {
        return CE_OK;
    }

    m_err = ce_errno;

    if (m_err == CE_ENOPATH) {

        if (!ext->create_path) {
            goto failed;
        }

        m_err = ce_create_full_path(to->data, ce_dir_access(ext->path_access));

        if (m_err) {
#if 0
	       	ce_log_error(CE_LOG_CRIT, ext->log, err,
                          ce_create_dir_n " \"%s\" failed", to->data);
#endif
            m_err = 0;
            goto failed;
        }

        if (ce_rename_file(src->data, to->data) != CE_FILE_ERROR) {
            return CE_OK;
        }

        m_err = ce_errno;
    }

    if (m_err == CE_EXDEV) {

        cf.size = -1;
        cf.buf_size = 0;
        cf.access = ext->access;
        cf.time = ext->time;
#if 0
       	cf.log = ext->log;
#endif
        name = (u_char*)malloc(to->len + 1 + 10 + 1/*,*ext->log*/);
        if (name == NULL) {
            return CE_ERROR;
        }

        (void) ce_sprintf(name, "%*s.%010uD%Z", to->len, to->data,
                           (uint32_t) ce_next_temp_number(0));

        if (ce_copy_file(src->data, name, &cf) == CE_OK) {

            if (ce_rename_file(name, to->data) != CE_FILE_ERROR) {
                ce_free(name);

                if (ce_delete_file(src->data) == CE_FILE_ERROR) {
#if 0    
		ce_log_error(CE_LOG_CRIT, ext->log, ce_errno,
                                  ce_delete_file_n " \"%s\" failed",
     				  src->data);
#endif    
 		return CE_ERROR;
                }

                return CE_OK;
            }
#if 0
            ce_log_error(CE_LOG_CRIT, ext->log, ce_errno,
                          ce_rename_file_n " \"%s\" to \"%s\" failed",
                          name, to->data);
#endif
            if (ce_delete_file(name) == CE_FILE_ERROR) {
#if 0
	  	    ce_log_error(CE_LOG_CRIT, ext->log, ce_errno,
                              ce_delete_file_n " \"%s\" failed", name);
#endif
            }
        }

        ce_free(name);

        m_err = 0;
    }

failed:

    if (ext->delete_file) {
        if (ce_delete_file(src->data) == CE_FILE_ERROR) {
#if 0
		ce_log_error(CE_LOG_CRIT, ext->log, ce_errno,
     			ce_delete_file_n " \"%s\" failed", src->data);
#endif
     	}
    }

    if (m_err) 
    {
#if 0
	    ce_log_error(CE_LOG_CRIT, ext->log, err,
                      ce_rename_file_n " \"%s\" to \"%s\" failed",
                      src->data, to->data);
#endif
    }

    return CE_ERROR;
}


ce_int_t ce_copy_file(u_char *from, u_char *to, ce_copy_file_t *cf)
{
    char             *buf;
    off_t             size;
    size_t            len;
    ssize_t           n;
    ce_fd_t          fd, nfd;
    ce_int_t         rc;
    ce_file_info_t   fi;

    rc = CE_ERROR;
    buf = NULL;
    nfd = CE_INVALID_FILE;

    fd = ce_open_file(from, CE_FILE_RDONLY, CE_FILE_OPEN, 0);

    if (fd == CE_INVALID_FILE) {
#if 0
	    ce_log_error(CE_LOG_CRIT, cf->log, ce_errno,
                      ce_open_file_n " \"%s\" failed", from);
#endif
	    goto failed;
    }

    if (cf->size != -1) {
        size = cf->size;

    } else {
        if (ce_fd_info(fd, &fi) == CE_FILE_ERROR) {
#if 0
	  	ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
                          ce_fd_info_n " \"%s\" failed", from);
#endif
            goto failed;
        }

        size = ce_file_size(&fi);
    }

    len = cf->buf_size ? cf->buf_size : 65536;

    if ((off_t) len > size) {
        len = (size_t) size;
    }

    buf =(char*)malloc(len/*, cf->log*/);
    if (buf == NULL) 
    {
        goto failed;
    }

    nfd = ce_open_file(to, CE_FILE_WRONLY, CE_FILE_CREATE_OR_OPEN,
                        cf->access);

    if (nfd == CE_INVALID_FILE) {
#if 0
	    ce_log_error(CE_LOG_CRIT, cf->log, ce_errno,
                      ce_open_file_n " \"%s\" failed", to);
#endif 
	    goto failed;
    }

    while (size > 0) 
    {

        if ((off_t) len > size) {
            len = (size_t) size;
        }

        n = ce_read_fd(fd, buf, len);

        if (n == CE_FILE_ERROR) 
	{
#if 0
		ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
    			ce_read_fd_n " \"%s\" failed", from);
#endif
		goto failed;
        }

        if ((size_t) n != len) {
#if 0
	  	ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
                          ce_read_fd_n " has read only %z of %uz from %s",
                          n, size, from);
#endif
	 	goto failed;
        }

        n = ce_write_fd(nfd, buf, len);

        if (n == CE_FILE_ERROR) {
#if 0
		ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
     			ce_write_fd_n " \"%s\" failed", to);
#endif
	 	goto failed;
        }

        if ((size_t) n != len) {
#if 0
	  	ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
                          ce_write_fd_n " has written only %z of %uz to %s",
                          n, size, to);
#endif
	 	goto failed;
        }

        size -= n;
    }

    if (cf->time != -1) {
        if (ce_set_file_time(to, nfd, cf->time) != CE_OK) {
#if 0
	  	ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
                          ce_set_file_time_n " \"%s\" failed", to);
#endif
	 	goto failed;
        }
    }

    rc = CE_OK;

failed:

    if (nfd != CE_INVALID_FILE) {
        if (ce_close_file(nfd) == CE_FILE_ERROR) {
#if 0
	  	ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
                          ce_close_file_n " \"%s\" failed", to);
#endif
     	}
    }

    if (fd != CE_INVALID_FILE) {
        if (ce_close_file(fd) == CE_FILE_ERROR) {
#if 0
	  	ce_log_error(CE_LOG_ALERT, cf->log, ce_errno,
                          ce_close_file_n " \"%s\" failed", from);
#endif
     	}
    }

    if (buf) {
        ce_free(buf);
    }

    return rc;
}
////////////////////////////////////////////////////////////////////////////////////
ssize_t ce_read_file(ce_file_t *file, u_char *buf, size_t size, off_t offset)
{
    ssize_t  n;
#if 0
    ce_log_debug4(CE_LOG_DEBUG_CORE, file->log, 0,
                   "read: %d, %p, %uz, %O", file->fd, buf, size, offset);
#endif
    n = pread(file->fd, buf, size, offset);

    if (n == -1) 
    {
#if 0    
    ce_log_error(CE_LOG_CRIT, file->log, ce_errno,
                      "pread() \"%s\" failed", file->name.data);
#endif    
    return CE_ERROR;
    }

    file->offset += n;

    return n;
}


ssize_t ce_write_file(ce_file_t *file, 
			u_char *buf, 
			size_t size, 
			off_t offset)
{
    ssize_t  n, written;
#if 0
    ce_log_debug4(CE_LOG_DEBUG_CORE, file->log, 0,
                   "write: %d, %p, %uz, %O", file->fd, buf, size, offset);
#endif
    written = 0;

    for ( ;; )
	{
        n = pwrite(file->fd, buf + written, size, offset);

       	if (n == -1) 
		{
#if 0
	       	ce_log_error(CE_LOG_CRIT, file->log, ce_errno,
                          "pwrite() \"%s\" failed", file->name.data);
#endif
	 	 	return CE_ERROR;
        }

        file->offset += n;
        written += n;

        if ((size_t) n == size) 
		{
            return written;
        }

        offset += n;
        size -= n;
    }
}


ce_fd_t ce_open_tempfile(u_char *name, ce_uint_t persistent,
			     ce_uint_t m_access)
{
    ce_fd_t  fd;

    fd = open((const char *) name, O_CREAT|O_EXCL|O_RDWR,
              m_access ? m_access : 0600);

    if (fd != -1 && !persistent) {
        unlink((const char *) name);
    }

    return fd;
}





ce_int_t ce_set_file_time(u_char *name, ce_fd_t fd, time_t s)
{
    fd=fd;
    struct timeval  tv[2];

    tv[0].tv_sec = ce_time();
    tv[0].tv_usec = 0;
    tv[1].tv_sec = s;
    tv[1].tv_usec = 0;

    if (utimes((char *) name, tv) != -1) {
        return CE_OK;
    }

    return CE_ERROR;
}

ce_int_t ce_create_file_mapping(ce_file_mapping_t *fm)
{
    fm->fd = ce_open_file(fm->name, CE_FILE_RDWR, CE_FILE_TRUNCATE,
                           CE_FILE_DEFAULT_ACCESS);
    if (fm->fd == CE_INVALID_FILE) {
#if 0
	    ce_log_error(CE_LOG_CRIT, fm->log, ce_errno,
                      ce_open_file_n " \"%s\" failed", fm->name);
#endif
	    return CE_ERROR;
    }

    if (ftruncate(fm->fd, fm->size) == -1) {
#if 0
	    ce_log_error(CE_LOG_CRIT, fm->log, ce_errno,
                      "ftruncate() \"%s\" failed", fm->name);
#endif
	    goto failed;
    }

    fm->addr = mmap(NULL, fm->size, PROT_READ|PROT_WRITE, MAP_SHARED,
                    fm->fd, 0);
    if (fm->addr != MAP_FAILED) {
        return CE_OK;
    }
#if 0
    ce_log_error(CE_LOG_CRIT, fm->log, ce_errno,
                  "mmap(%uz) \"%s\" failed", fm->size, fm->name);
#endif

failed:

    if (ce_close_file(fm->fd) == CE_FILE_ERROR) {
#if 0
	    ce_log_error(CE_LOG_ALERT, fm->log, ce_errno,
                      ce_close_file_n " \"%s\" failed", fm->name);
#endif
    }

    return CE_ERROR;
}


void ce_close_file_mapping(ce_file_mapping_t *fm)
{
    if (munmap(fm->addr, fm->size) == -1) {
#if 0
	    ce_log_error(CE_LOG_CRIT, fm->log, ce_errno,
                      "munmap(%uz) \"%s\" failed", fm->size, fm->name);
#endif
    }

    if (ce_close_file(fm->fd) == CE_FILE_ERROR) {
#if 0
	    ce_log_error(CE_LOG_ALERT, fm->log, ce_errno,
                      ce_close_file_n " \"%s\" failed", fm->name);
#endif
    }
}


ce_int_t ce_open_dir(ce_str_t *name, ce_dir_t *dir)
{
    dir->dir = opendir((const char *) name->data);

    if (dir->dir == NULL) {
        return CE_ERROR;
    }

    dir->valid_info = 0;

    return CE_OK;
}


ce_int_t ce_read_dir(ce_dir_t *dir)
{
    dir->de = readdir(dir->dir);

    if (dir->de) {

        dir->type = 0;

        return CE_OK;
    }

    return CE_ERROR;
}


ce_int_t ce_open_glob(ce_glob_t *gl)
{
    int  n;

    n = glob((char *) gl->pattern, GLOB_NOSORT, NULL, &gl->pglob);

    if (n == 0) {
        return CE_OK;
    }

    return CE_ERROR;
}


ce_int_t ce_read_glob(ce_glob_t *gl, ce_str_t *name)
{
    size_t  count;

    count = (size_t) gl->pglob.gl_pathc;

    if (gl->n < count) {

        name->len = (size_t) ce_strlen(gl->pglob.gl_pathv[gl->n]);
        name->data = (u_char *) gl->pglob.gl_pathv[gl->n];
        gl->n++;

        return CE_OK;
    }

    return CE_DONE;
}


void ce_close_glob(ce_glob_t *gl)
{
    globfree(&gl->pglob);
}


ce_err_t ce_trylock_fd(ce_fd_t fd)
{
    struct flock  fl;

    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        return ce_errno;
    }

    return 0;
}


ce_err_t ce_lock_fd(ce_fd_t fd)
{
    struct flock  fl;

    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        return ce_errno;
    }

    return 0;
}


ce_err_t ce_unlock_fd(ce_fd_t fd)
{
    struct flock  fl;

    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        return  ce_errno;
    }

    return 0;
}

size_t ce_fs_bsize(u_char *name)
{
    name=name;
    return 512;
}

ce_int_t ce_filepath_root(const char **rootpath,const char **inpath,int32_t flags,ce_pool_t *p)
{
   if (**inpath == '/') {
        *rootpath = ce_pstrdup(p, "/");
        do {
            ++(*inpath);
        } while (**inpath == '/');

        return CE_OK;
    }

    return CE_ERROR;
}

ssize_t ce_file_getc(ce_file_t *file,char *ch){
	
	return ce_read_file(file,ch,1,file->offset);
}

ssize_t ce_file_gets(ce_file_t *thefile,char* str,size_t len){

    ssize_t rv = 0; /* get rid of gcc warning */
    size_t nbytes;
    const char *str_start = str;
    char *final = str + len - 1;

    if (len <= 1) {  
        /* sort of like fgets(), which returns NULL and stores no bytes 
         */
        return rv;
    }

    while (str < final) { /* leave room for trailing '\0' */
            nbytes = 1;
            rv = ce_read_file(thefile, str, nbytes,thefile->offset);
            if (rv == CE_ERROR) {
            	return CE_ERROR;
	    }
	    if(rv==0)
	    {
		    break;
		    }

            if (*str == '\n') {
                ++str;
                break;
            }
            ++str;
    }

    /* We must store a terminating '\0' if we've stored any chars. We can
     * get away with storing it if we hit an error first. 
     */
    *str = '\0';
     return rv;
}
