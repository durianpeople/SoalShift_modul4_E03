#define FUSE_USE_VERSION 28
#define CIPHERMAX 94
#include <fuse.h>
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

static const char *mountable = "/home/durianpeople/Documents/Notes/SISOP/REPO/mountable";
static const char *mountpoint = "/home/durianpeople/Documents/Notes/SISOP/REPO/mount_point";
const int encryption_key = 17;

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

void cipherString(const char *input, char *output, int key)
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

//fungsi xmp_*()

void *xmp_init(struct fuse_conn_info *conn) //sebelum mount
{
    //NO 2
    char target[1000];
    char encrypted_foldername[1000] = "";
    cipherString("/Videos", encrypted_foldername, encryption_key);
    sprintf(target, "%s%s", mountable, encrypted_foldername);
    struct stat st = {0};
    printf("Create folder %s\n", target);
    if (stat(target, &st) == -1)
    {
        mkdir(target, 0700);
    }
    return 0;
}
void xmp_destroy(void *private_data) //sebelum unmount
{
    //NO 2
    char target[1000];
    char encrypted_foldername[1000] = "";
    cipherString("/Videos", encrypted_foldername, encryption_key);
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
    cipherString(path, encrypted_path, encryption_key);
    sprintf(fpath, "%s%s", mountable, encrypted_path);
    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
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
            cipherString(de->d_name, decrypted_name, CIPHERMAX - encryption_key);

        res = (filler(buf, decrypted_name, &st, 0));
        if (res != 0)
            break;
    }

    closedir(dp);
    return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = creat(fpath, mode);
    if (res == -1)
        res = -errno;

    return res;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    res = mknod(fpath, mode, rdev);
    if (res == -1)
        res = -errno;

    return res;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
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

    return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    int fd;
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
    char fpath[1000];
    if (strcmp(path, "/") == 0)
    {
        path = mountable;
        sprintf(fpath, "%s", path);
    }
    else
        sprintf(fpath, "%s%s", mountable, encrypted_path);

    fd = open(fpath, O_WRONLY);
    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    int res;
    char fpath[1000];
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
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
    cipherString(path, encrypted_path, encryption_key);
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

    return res;
}

static int xmp_rmdir(const char *path)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(path, encrypted_path, encryption_key);
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

    return res;
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
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
    // CAESAR CIPHER
    // char input[1000];
    // scanf("%s", input);
    // char output[1000] = "";
    // cipherString(input, output, encryption_key);
    // printf("%s\n", output);
    return 0;
}