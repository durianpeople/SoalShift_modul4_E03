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
#include <pwd.h>
#include <grp.h>
#include <time.h>

static const char *mountable = "/home/durianpeople/Documents/Notes/SISOP/REPO/mountable";
static const char *mount_point = "/home/durianpeople/Documents/Notes/SISOP/REPO/mount_point";
const int encryption_key = 17;
int bypass_mkv = 1;
int ignore_backup = 0;

pthread_t mergeThreadID;
pthread_t youtuberFileThreadID;
pthread_t backupThreadID;
char path_transport[1000] = "";
char outputcipher[2] = "";
char *cipher(char input, int key)
{
    char charset[] = {113, 69, 49, 126, 32, 89, 77, 85, 82, 50, 34, 96, 104, 78, 73, 100, 80, 122, 105, 37, 94, 116, 64, 40, 65, 111, 58, 61, 67, 81, 44, 110, 120, 52, 83, 91, 55, 109, 72, 70, 121, 101, 35, 97, 84, 54, 43, 118, 41, 68, 102, 75, 76, 36, 114, 63, 98, 107, 79, 71, 66, 62, 125, 33, 57, 95, 119, 86, 39, 93, 106, 99, 112, 53, 74, 90, 38, 88, 108, 124, 92, 56, 115, 59, 103, 60, 123, 51, 46, 117, 42, 87, 45, 48}; //length: 94
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
    printf("Merge thread created\n");
    struct dirent **namelist;
    bypass_mkv = 0;
    int n = scandir(mount_point, &namelist, 0, alphasort);
    bypass_mkv = 1;
    if (n < 0)
        perror("scandir");
    else
    {
        char destpath[2000] = "";
        char destname[1000] = "";
        char sourcepath[1000] = "";
        for (int ii = 0; ii < n; ii++)
        {
            // printf("Checking: %s\n", namelist[ii]->d_name);
            char tmpname[1000] = "";
            get_filename_name(namelist[ii]->d_name, tmpname);

            // printf("\tTmpname: %s\n", tmpname);
            // printf("\tDestname: %s\n", destname);
            if (strcmp(get_filename_ext(namelist[ii]->d_name), "000") == 0 &&
                strcmp(get_filename_ext(tmpname), "mkv") == 0 &&
                namelist[ii]->d_type != 4)
            {
                sprintf(destpath, "%s/Videos/%s", mount_point, tmpname);
                strcpy(destname, tmpname);
                sprintf(sourcepath, "%s/%s", mount_point, namelist[ii]->d_name);
            }
            else if (strcmp(tmpname, destname) == 0)
            {
                sprintf(sourcepath, "%s/%s", mount_point, namelist[ii]->d_name);
            }
            else
                strcpy(destpath, "");
            if (strcmp(destpath, "") != 0)
            {
                // printf("Merging %s\n", sourcepath);
                // printf("To %s\n", destpath);
                ignore_backup = 1;
                FILE *source = fopen(sourcepath, "rb");
                FILE *dest = fopen(destpath, "ab+");

                char c = fgetc(source);
                while (c != EOF)
                {
                    fputc(c, dest);
                    c = fgetc(source);
                }

                fclose(source);
                fclose(dest);
                ignore_backup = 0;
            }
            free(namelist[ii]);
        }
        free(namelist);
    }
    printf("Merge thread finished\n");
    return 0;
}

void *youtuberFileThread(void *arg)
{
    printf("Youtuber file thread created\n");
    char *path = (char *)arg;
    char fpathfrom[1000];
    char fpathto[1000];
    sprintf(fpathfrom, "%s%s", mount_point, path);
    sprintf(fpathto, "%s%s.iz1", mount_point, path);
    chmod(fpathfrom, S_IRUSR | S_IWUSR | S_IRGRP);
    rename(fpathfrom, fpathto);
    printf("Youtuber file thread finished\n");
    return 0;
}

void *backupThread(void *arg)
{
    printf("Backup thread created\n");

    printf("Backup thread finished\n");
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
        printf("Failed to create merge thread\n");
    return 0;
}
void xmp_destroy(void *private_data) //sebelum unmount
{
    //NO 2
    printf("Waiting for thread...\n");
    pthread_join(mergeThreadID, NULL);
    pthread_join(youtuberFileThreadID, NULL);
    pthread_join(backupThreadID, NULL);

    printf("Deleting files...\n");
    DIR *dp;
    struct dirent *de;

    char encrypted_foldername[1000] = "";
    cipherString(encrypted_foldername, "/Videos", encryption_key);
    char target[1000];
    sprintf(target, "%s%s", mountable, encrypted_foldername);
    dp = opendir(target);
    while ((de = readdir(dp)) != NULL)
    {
        char spath[2000];
        sprintf(spath, "%s/%s", target, de->d_name);
        remove(spath);
    }
    closedir(dp);

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
        //NO 1
        char decrypted_name[1000] = "";
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            strcpy(decrypted_name, de->d_name);
        else
            cipherString(decrypted_name, de->d_name, CIPHERMAX - encryption_key);

        //NO 3
        struct group *grd;
        struct passwd *pws;
        struct stat buf2;
        char statString[2000];
        sprintf(statString, "%s/%s", fpath, de->d_name);
        stat(statString, &buf2);
        pws = getpwuid(buf2.st_uid);
        grd = getgrgid(buf2.st_gid);
        struct tm *info;
        char atime_string[80];
        time_t a_time;
        a_time = buf2.st_atime;
        info = localtime(&a_time);
        strftime(atime_string, 80, "%x - %I:%M%p", info);
        // printf("%s\n File Permissions: \t", statString);
        // printf("%s\n", pws->pw_name);
        // printf("%s\n", grd->gr_name);
        // printf("\n");
        if (strcmp(pws->pw_name, "chipset") == 0 && strcmp(grd->gr_name, "rusak") == 0 && !(buf2.st_mode & S_IRUSR & S_IRGRP & S_IROTH))
        {
            printf("FILE BAHAYA %s\n", de->d_name);
            char filemiris[1000] = "";
            cipherString(filemiris, "/filemiris.txt", encryption_key);
            char ffilemiris[2000] = "";
            sprintf(ffilemiris, "%s%s", mountable, filemiris);
            FILE *Ffilemiris = fopen(ffilemiris, "a+");
            fprintf(Ffilemiris, "%s %s %s %s\n", decrypted_name, pws->pw_name, grd->gr_name, atime_string);
            fclose(Ffilemiris);
            remove(statString);
        }

        //sisanya
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

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
    sprintf(fpath, "%s%s", mountable, encrypted_path);

    (void)fi;
    res = creat(fpath, mode);
    if (res == -1)
        res = -errno;
    close(res);
    strcpy(path_transport, path);
    char *ptr = strstr(path, "/YOUTUBER");
    if (ptr != NULL && strcmp(path, "/YOUTUBER") != 0 && ptr - path == 0)
    {
        if (pthread_create(&youtuberFileThreadID, NULL, &youtuberFileThread, path_transport) != 0)
            printf("Failed to create Youtuber file thread\n");
    }
    return 0;
}

static int xmp_rename(const char *from, const char *to)
{
    int res;
    //NO 1
    char encrypted_from[1000] = "";
    char encrypted_to[1000] = "";
    cipherString(encrypted_from, from, encryption_key);
    cipherString(encrypted_to, to, encryption_key);
    char ffrom[1000];
    char fto[1000];
    if (strcmp(from, "/") == 0)
    {
        from = mountable;
        sprintf(ffrom, "%s", from);
    }
    else
        sprintf(ffrom, "%s%s", mountable, encrypted_from);
    if (strcmp(to, "/") == 0)
    {
        to = mountable;
        sprintf(fto, "%s", to);
    }
    else
        sprintf(fto, "%s%s", mountable, encrypted_to);
    res = rename(ffrom, fto);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_chmod(const char *path, mode_t mode) //template approved
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
    sprintf(fpath, "%s%s", mountable, encrypted_path);

    if (strcmp(get_filename_ext(path), "iz1") != 0)
        res = chmod(fpath, mode);
    else
    {
        printf("File ekstensi iz1 tidak boleh diubah permission-nya\n");
        return -EACCES;
    }
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
    sprintf(fpath, "%s%s", mountable, encrypted_path);

    (void)fi;
    fd = open(fpath, O_WRONLY);
    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    if (strcmp(get_filename_ext(path), "swp") != 0 && !ignore_backup)
    {
        strcpy(path_transport, path);
        if (pthread_create(&backupThreadID, NULL, &backupThread, path_transport) != 0)
            printf("Failed to create merge thread\n");
    }
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
    char *ptr = strstr(path, "/YOUTUBER");
    if (ptr != NULL && strcmp(path, "/YOUTUBER") != 0 && ptr - path == 0)
    {
        chmod(fpath, S_IRWXU | S_IRGRP | S_IXGRP);
    }
    return 0;
}

static int xmp_unlink(const char *path)
{
    int res;
    //NO 1
    char encrypted_path[1000] = "";
    cipherString(encrypted_path, path, encryption_key);
    char fpath[1000];
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
    .rename = xmp_rename,
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