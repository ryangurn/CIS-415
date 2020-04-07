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

#include "command.h"

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
  if (!directory) {
    fprintf(stdout, "Error!: Cannot open directory\n");
  }

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

  // open the directory
  DIR *directory = opendir(current_working_directory);
  if (!directory) {
    fprintf(stdout, "Error!: Cannot open directory\n");
  }

  // read the Directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(destinationPath, dentry->d_name) == 0)
    {
      fprintf(stdout, "Error!: Given Directory already exists\n");

      // close the Directory
      closedir(directory);
      return;
    }
  } while((dentry = readdir(directory)));

  closedir(directory); // close the directory

  int source_path = open(sourcePath, O_RDONLY);
  int destination_path = open(destinationPath, O_WRONLY | O_CREAT, S_IRWXU);
  if (source_path == -1)
  {
    fprintf(stdout, "Error!: Issue opening source file\n");
    close(destination_path);
    close(source_path);
    return;
  }

  char buf[PATH_MAX]; // buffer
  int char_count = read(source_path, buf, PATH_MAX);
  do {
    write(destination_path, buf, char_count);
  } while((char_count = read(source_path, buf, PATH_MAX)) > 0);


  close(source_path);
  close(destination_path);

  return;

}

void moveFile(char *sourcePath, char *destinationPath)
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
  if (!directory) {
    fprintf(stdout, "Error!: Cannot open directory\n");
  }

  // read the Directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(destinationPath, dentry->d_name) == 0)
    {
      fprintf(stdout, "Error!: Given Directory already exists\n");

      // close the Directory
      closedir(directory);
      return;
    }
  } while((dentry = readdir(directory)));

  closedir(directory); // close the directory

  int source_path = open(sourcePath, O_RDONLY);
  int destination_path = open(destinationPath, O_WRONLY | O_CREAT, S_IRWXU);
  if (source_path == -1)
  {
    fprintf(stdout, "Error!: Issue opening source file\n");
    close(destination_path);
    close(source_path);
    return;
  }

  char buf[PATH_MAX]; // buffer
  int char_count = read(source_path, buf, PATH_MAX);
  do {
    write(destination_path, buf, char_count);
  } while((char_count = read(source_path, buf, PATH_MAX)) > 0);


  close(source_path);
  close(destination_path);
  deleteFile(sourcePath); // delete origional file
  return;
}

void deleteFile(char *filename)
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
  if (!directory) {
    fprintf(stdout, "Error!: Cannot open directory\n");
  }

  // read the Directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(filename, dentry->d_name) == 0)
    {
      int deleted = unlink(filename);
      if(deleted == -1)
      {
        fprintf(stdout, "Error!: Cannot delete given file\n");
        return;
      }

      // close the Directory
      closedir(directory);
      return;
    }
  } while((dentry = readdir(directory)));

  closedir(directory); // close the directory

  fprintf(stdout, "Error!: No such file exists\n");
  return;

}

void displayFile(char *filename)
{
  int file_path = open(filename, O_RDONLY);
  if (file_path == -1)
  {
    fprintf(stdout, "Error!: Issue opening file\n");
    close(file_path);
    return;
  }

  char buf[PATH_MAX];
  int char_count = read(file_path, buf, PATH_MAX);
  do {
    fprintf(stdout, buf, char_count);
  } while((char_count = read(file_path, buf, PATH_MAX)) > 0);

  close(file_path);
  return;
}
