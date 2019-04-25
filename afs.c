#define _GNU_SOURCE
#define FUSE_USE_VERSION 28
#define CIPHERMAX 94
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fuse.h>
#include <pthread.h>
#include <sys/wait.h>

static const char *mountable = "/home/durianpeople/Documents/Notes/SISOP/REPO/mountable";
const int encryption_key = 17;
const int bypass_mkv = 0;

pthread_t mergeThreadID;
char outputcipher[2] = "";
char *cipher(char input, int key)
{
    char charset[] = {113,
                      69,
                      49,
                      126,
                      32,
                      89,
                      77,
                      85,
                      82,
                      50,
                      34,
                      96,
                      104,
                      78,
                      73,
                      100,
                      80,
                      122,
                      105,
                      37,
                      94,
                      116,
                      64,
                      40,
                      65,
                      111,
                      58,
                      61,
                      67,
                      81,
                      44,
                      110,
                      120,
                      52,
                      83,
                      91,
                      55,
                      109,
                      72,
                      70,
                      121,
                      101,
                      35,
                      97,
                      84,
                      54,
                      43,
                      118,
                      41,
                      68,
                      102,
                      75,
                      76,
                      36,
                      114,
                      63,
                      98,
                      107,
                      79,
                      71,
                      66,
                      62,
                      125,
                      33,
                      57,
                      95,
                      119,
                      86,
                      39,
                      93,
                      106,
                      99,
                      112,
                      53,
                      74,
                      90,
                      38,
                      88,
                      108,
                      124,
                      92,
                      56,
                      115,
                      59,
                      103,
                      60,
                      123,
                      51,
                      46,
                      117,
                      42,
                      87,
                      45,
                      48}; //length: 94
    int i;
    for (i = 0; i < 94; i++)
        if (charset[i] == input)
        {
            outputcipher[0] = charset[(i + key) % 94];
            return outputcipher;
        }
    outputcipher[0] = input;
    return outputcipher;
}

void cipherString(char *output, const char *input, int key)
{
    for (int i = 0; i < strlen(input); i++)
    {
        strcat(output, cipher(input[i], key));
    }
}

//NO 2
const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

void get_filename_name(const char *filename, char *target)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename || dot == NULL)
        strcpy(target, filename);
    else
        strncpy(target, filename, strlen(filename) - strlen(dot) - 0);
}

static int stringCompare(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void sortString(const char *arr[], int n)
{
    qsort(arr, n, sizeof(const char *), stringCompare);
}

void *mergeThread(void *arg)
{
    printf("Thread created\n");
    // struct dirent **namelist;
    // int n = scandir(".", &namelist, 0, alphasort);
    // if (n < 0)
    //     perror("scandir");
    // else
    // {
    //     while (n--)
    //     {
    //         printf("%s\n", namelist[n]->d_name);
    //         free(namelist[n]);
    //     }
    //     free(namelist);
    // }
    return 0;
}

//fungsi xmp_*()

void *xmp_init(struct fuse_conn_info *conn) //sebelum mount
{
    //NO 2
    char target[1000];
    char encrypted_foldername[1000] = "";
    cipherString(encrypted_foldername, "/Videos", encryption_key);
    sprintf(target, "%s%s", mountable, encrypted_foldername);
    struct stat st = {0};
    printf("Create folder %s\n", target);
    if (stat(target, &st) == -1)
    {
        mkdir(target, 0700);
    }

    if (pthread_create(&mergeThreadID, NULL, &mergeThread, NULL) != 0)
        printf("Failed to create thread\n");
    return 0;
}
void xmp_destroy(void *private_data) //sebelum unmount
{
    //NO 2
    printf("Waiting for thread...\n");
    pthread_join(mergeThreadID, NULL);
    char target[1000];
    char encrypted_foldername[1000] = "";
    cipherString(encrypted_foldername, "/Videos", encryption_key);
    sprintf(target, "%s%s", mountable, encrypted_foldername);
    printf("Destroy folder %s\n", target);
    rmdir(target);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    sprintf(fpath, "%s%s", mountable, encrypted_path);
    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) //clear
{
    char fpath[1000];
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    if (strcmp(path, "/") == 0)
    {
        strcpy(encrypted_path, mountable);
        sprintf(fpath, "%s", encrypted_path);
    }
    else if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        sprintf(fpath, "%s", path);
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);
    int res = 0;

    DIR *dp;
    struct dirent *de;

    (void)offset;
    (void)fi;

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    //NO 2
    char excluded[1000] = "";
    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        //NO 1
        char decrypted_name[1000] = "";
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            strcpy(decrypted_name, de->d_name);
        else
            cipherString(decrypted_name, de->d_name, CIPHERMAX - encryption_key);

        char tmpfilename[1000] = "";
        get_filename_name(decrypted_name, tmpfilename);
        if (strcmp(get_filename_ext(tmpfilename), "mkv") == 0 && de->d_type != 4)
        {
            strcpy(excluded, tmpfilename);
        }
        if (bypass_mkv)
        {
            if (strcmp(excluded, tmpfilename) != 0)
            {
                res = (filler(buf, decrypted_name, &st, 0));
                if (res != 0)
                    break;
            }
        }
        else
        {
            res = (filler(buf, decrypted_name, &st, 0));
            if (res != 0)
                break;
        }
    }

    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) //template approved
{
    int fd;
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    (void)fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi) //template approved
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    (void)fi;
    res = creat(fpath, mode);
    if (res == -1)
        res = -errno;
    close(res);
    return 0;
}

static int xmp_chmod(const char *path, mode_t mode) //template approved
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = chmod(fpath, mode);
    if (res == -1)
        res = -errno;
    return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);
    res = lchown(fpath, uid, gid);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);
    res = statvfs(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
    /* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

    (void)path;
    (void)fi;
    return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
                     struct fuse_file_info *fi)
{
    /* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

    (void)path;
    (void)isdatasync;
    (void)fi;
    return 0;
}

int xmp_truncate(const char *path, off_t size) //template approved
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = truncate(fpath, size);
    if (res == -1)
        res = -errno;
    return 0;
}

int xmp_utimens(const char *path, const struct timespec ts[2]) //template approved
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);
    struct timeval tv[2];
    tv[0].tv_sec = ts[0].tv_sec;
    tv[0].tv_usec = ts[0].tv_nsec / 1000;
    tv[1].tv_sec = ts[1].tv_sec;
    tv[1].tv_usec = ts[1].tv_nsec / 1000;

    res = utimes(path, tv);
    if (res == -1)
        res = -errno;
    return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev) //template approved
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    if (S_ISREG(mode))
    {
        res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (res >= 0)
            res = close(res);
    }
    else if (S_ISFIFO(mode))
        res = mkfifo(fpath, mode);
    else
        res = mknod(fpath, mode, rdev);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) //template approved?
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = open(fpath, fi->flags);
    if (res == -1)
        res = -errno;
    close(res);
    return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi) //template approved
{
    int fd;
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    (void)fi;
    fd = open(fpath, O_WRONLY);
    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int xmp_mkdir(const char *path, mode_t mode) //template approved
{
    int res;
    char fpath[1000];
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    sprintf(fpath, "%s%s", mountable, encrypted_path);
    res = mkdir(fpath, mode);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_unlink(const char *path)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = unlink(fpath);
    if (res == -1)
        res = -errno;

    return 0;
}

static int xmp_rmdir(const char *path)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = rmdir(fpath);
    if (res == -1)
        res = -errno;

    return 0;
}

static struct fuse_operations xmp_oper = {
    //
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .init = xmp_init,
    .destroy = xmp_destroy,
    .mkdir = xmp_mkdir,
    .write = xmp_write,
    .create = xmp_create,
    .mknod = xmp_mknod,
    .open = xmp_open,
    .unlink = xmp_unlink,
    .rmdir = xmp_rmdir,
    .chmod = xmp_chmod,
    .truncate = xmp_truncate,
    .utimens = xmp_utimens,
    .read = xmp_read,
    .release = xmp_release,
    .statfs = xmp_statfs,
    .chown = xmp_chown,
    .fsync = xmp_fsync,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
    // CAESAR CIPHER
    // char input[1000];
    // scanf("%s", input);
    // char output[1000] = "";
    // cipherString( output, input,encryption_key);
    // printf("%s\n", output);
    return 0;
}