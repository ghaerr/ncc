typedef struct __dirent_dir DIR;

#ifdef __APPLE__
#if 1
struct dirent {         /* getdirentries64 */
        unsigned long  d_ino;           /* file number of entry */
        unsigned long  d_seekoff;
        unsigned short d_reclen;        /* length of this record */
        unsigned char  d_namlen;        /* length of string in d_name */
        unsigned char  d_type;          /* file type, see below */
        unsigned char  d_namlen2;
        char           d_name[255];     /* name must be no longer than this */
};
#else
struct dirent {         /* getdirentries */
        unsigned int   d_ino;           /* file number of entry */
        unsigned short d_reclen;        /* length of this record */
        unsigned char  d_type;          /* file type, see below */
        unsigned char  d_namlen;        /* length of string in d_name */
        char           d_name[256];     /* name must be no longer than this */
};
#endif
#else
struct dirent {
        unsigned long d_ino;
        unsigned long d_off;
        unsigned short d_reclen;
        char d_name[256];
};
#endif

DIR *opendir(char *path);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
