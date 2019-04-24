#define FUSE_USE_VERSION 28
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

static const char *dirpath = "/home/durianpeople/Documents/Notes/SISOP/REPO/mountable";

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

//fungsi xmp_*()
static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;

    res = lstat(path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;

    (void)offset;
    (void)fi;

    dp = opendir(path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

static struct fuse_operations xmp_oper = {
    //
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
    // CAESAR CIPHER
    // char input[1000];
    // scanf("%s", input);
    // char output[1000] = "";
    // for (int i = 0; i < strlen(input); i++)
    // {
    //     strcat(output, cipher(input[i], 17));
    // }
    // printf("%s\n", output);
    // return 0;
}