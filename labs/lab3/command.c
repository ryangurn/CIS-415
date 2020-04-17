#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <linux/limits.h> // path_max
#include <ctype.h>

#include "command.h"

void lfcat()
{
  char buf[PATH_MAX];
  char current_working_directory[PATH_MAX];
  getcwd(current_working_directory, sizeof(current_working_directory));

  DIR *directory = opendir(current_working_directory);
  if(directory == NULL)
  {
    write(1, "Error!: Error showing current directory\n", sizeof("Error!: Error showing current directory\n"));
    return;
  }

  struct dirent **dentry;
  int n = scandir(".", &dentry, NULL, alphasort);
  int i = 0;
  while(i < n) {
    char *filename = dentry[i]->d_name;
    if (strcmp(filename, "Makefile") != 0 &&
        strcmp(filename, ".") != 0 &&
        strcmp(filename, "..") != 0 &&
        strcmp(filename, "a.out") != 0 &&
        strcmp(filename, "main.c") != 0 &&
        strcmp(filename, "main.o") != 0 &&
        strcmp(filename, "output.txt") != 0 &&
        strcmp(filename, "command.h") != 0 &&
        strcmp(filename, "command.c") != 0 &&
        strcmp(filename, "command.o") != 0 &&
        strcmp(filename, "command.h.gch") != 0 &&
        strcmp(filename, "input.txt") != 0)
    {
      write(1, "File: ", sizeof(char)*strlen("File: "));
      write(1, filename, sizeof(char)*strlen(filename));
      write(1, "\n", sizeof("\n"));

      // cat the file
      int file_path = open(filename, O_RDONLY);
      if (file_path == -1)
      {
        write(1, "Error!: Issue opening file\n", sizeof(char)*strlen("Error!: Issue opening file\n"));
        close(file_path);
        return;
      }

      int char_count = read(file_path, buf, PATH_MAX);
      do {
        write(1, buf, char_count);
      } while((char_count = read(file_path, buf, PATH_MAX)) > 0);

      close(file_path);
      write(1, "\n---------------------------------------------------------------------------------------------------------------\n", sizeof("\n---------------------------------------------------------------------------------------------------------------\n"));
    }
    free(dentry[i]);
    ++i;
  }
  free(dentry);

  closedir(directory);
  return;
}
