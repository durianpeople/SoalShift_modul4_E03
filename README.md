 # Laporan

Fixed constant:

```c
#define CIPHERMAX 94
```

Modifiable constant:

```c
static const char *mountable = "/path/ke/termount";
static const char *mount_point = "/path/ke/mount/point";
const int encryption_key = 17;
int bypass_mkv = 1;
int ignore_backup = 0;
```

**CATATAN**: Semua fungsi FUSE yang diperlukan telah diimplementasikan dalam source code **afs.c**.

## SOAL 1

Fungsi untuk enkripsi setiap karakter:

```c
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
```

Fungsi untuk enkripsi string

```c
void cipherString(char *output, const char *input, int key)
{
    for (int i = 0; i < strlen(input); i++)
    {
        strcat(output, cipher(input[i], key));
    }
}
```

Untuk dekripsi, menjalankan

```c
cipherString(output, input, CIPHERMAX - encryption_key);
```

Semua implementasi `xmp_*()` yang melibatkan argumen `const char *path`, maka dienkripsi dan dimasukkan ke `char *fpath`:

```c
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
```

Khusus di dalam `xmp_readdir()`, di dalam looping `readdir(dp)`, lakukan dekripsi

```c
char decrypted_name[1000] = "";
if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
    strcpy(decrypted_name, de->d_name);
else
    cipherString(decrypted_name, de->d_name, CIPHERMAX - encryption_key);
```

## SOAL 2

Implementasi fungsi sebelum mount berada di `xmp_init()`:

- Membuat folder:

  ```c
  if (stat(target, &st) == -1)
  {
      mkdir(target, 0700);
  }
  ```

- Membuat thread untuk merge video:

  ```c
  if (pthread_create(&mergeThreadID, NULL, &mergeThread, NULL) != 0)
      printf("Failed to create merge thread\n");
  ```

Dalam thread `mergeThread()`:

- Mengambil nama file dalam `mount_point` kemudian dimasukkan ke dalam `**namelist`:

  ```c
  struct dirent **namelist;
  bypass_mkv = 0; //agar video.mkv.*** muncul
  int n = scandir(mount_point, &namelist, 0, alphasort);
  bypass_mkv = 1;
  ```

- Untuk setiap file, cek apakah merupakan pecahan video dan merge:

  ```c
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
  ```

Di `readdir()`, jangan tampilkan pecahan video:

```c
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
```

Sebelum unmount, hapus seluruh **file** dalam folder Video, kemudian hapus folder tersebut:

```c
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
```

## SOAL 3

Pada loop readdir di `xmp_readdir()`, ambil semua informasi yang diperlukan:

```c
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
```

Kemudian deteksi jika file memenuhi kriteria, lalu lakukan tindakan terkait:

```c
if ((strcmp(pws->pw_name, "chipset") == 0 || strcmp(pws->pw_name, "ic_controller") == 0) && strcmp(grd->gr_name, "rusak") == 0 && !(buf2.st_mode & S_IRUSR & S_IRGRP & S_IROTH))
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
```

## SOAL 4

Di `xmp_mkdir()`, jika `*path` berada dalam folder YOUTUBER, maka `chmod()`:

```c
char *ptr = strstr(path, "/YOUTUBER");
if (ptr != NULL && strcmp(path, "/YOUTUBER") != 0 && ptr - path == 0)
{
    chmod(fpath, S_IRWXU | S_IRGRP | S_IXGRP);
}
```

Di `xmp_create()`, jika berada dalam folder YOUTUBER maka jalankan thread untuk rename dan chmod file:

```c
strcpy(path_transport, path);
char *ptr = strstr(path, "/YOUTUBER");
if (ptr != NULL && strcmp(path, "/YOUTUBER") != 0 && ptr - path == 0)
{
    if (pthread_create(&youtuberFileThreadID, NULL, &youtuberFileThread, path_transport) != 0)
        printf("Failed to create Youtuber file thread\n");
}
```

