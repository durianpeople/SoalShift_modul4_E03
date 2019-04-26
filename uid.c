struct group *grd;
struct passwd *pws;
struct stat buf2;
char statString[1000];
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
    sprintf(statString, "%s/%s", fpath, de->d_name);
    stat(statString, &buf2);
    pws = getpwuid(buf2.st_uid);
    grd = getgrgid(buf2.st_gid);

    // printf("%d\n", buf2.st_uid);
    // printf("%d\n", buf2.st_gid);
    // printf("%d\n", buf2.st_atime);

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

    printf("%s\n", decrypted_name);
    // printf("\t%s\n", pws->pw_name);
    // printf("\t%s\n", grd->gr_name);

    res = (filler(buf, decrypted_name, &st, 0));
    if (res != 0)
        break;
}

closedir(dp);
return 0;
}