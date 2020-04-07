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

void listDir()
{
  char current_working_directory[PATH_MAX];
  getcwd(current_working_directory, sizeof(current_working_directory));

  DIR *directory = opendir(current_working_directory);
  if(directory == NULL)
  {
    fprintf(stdout, "Error!: Error showing current directory\n");
    return;
  }

  struct dirent *dentry = readdir(directory);
  do {
    char *filename = dentry->d_name;
    printf("%s\n", filename);
  } while((dentry = readdir(directory)));

  closedir(directory);
  return;

}

void showCurrentDir()
{
  char current_working_directory[PATH_MAX];
  char *output = getcwd(current_working_directory, sizeof(current_working_directory));

  // error
  if (output == NULL)
  {
    fprintf(stdout, "Error!: Error showing current directory\n");
    return;
  }

  fprintf(stdout, "%s\n", current_working_directory);
  return;
}

void makeDir(char *dirName)
{
  // get the current working directory
  char current_working_directory[PATH_MAX];
  char *output = getcwd(current_working_directory, sizeof(current_working_directory));

  // error
  if (output == NULL)
  {
    fprintf(stdout, "Error!: Error showing current directory\n");
    return;
  }

  // open the directory
  DIR *directory = opendir(current_working_directory);

  // read the directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(dirName, dentry->d_name) == 0)
    {
      fprintf(stdout, "Error!: Directory already exists\n");

      // close the Directory
      closedir(directory);
      return;
    }
  } while((dentry = readdir(directory)));

  // close the Directory
  closedir(directory);
  mkdir(dirName, 0755);
  return;
}

void changeDir(char *dirName)
{
  int changed = chdir(dirName);

  if (changed != 0 )
  {
    fprintf(stdout, "Error!: Unable to change to given directory\n");
  }

  return;
}

void copyFile(char *sourcePath, char *destinationPath)
{
  // get the current working directory
  char current_working_directory[PATH_MAX];
  char *output = getcwd(current_working_directory, sizeof(current_working_directory));

  // error
  if (output == NULL)
  {
    fprintf(stdout, "Error!: Error showing current directory\n");
    return;
  }
}
