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

void listDir()
{
  char current_working_directory[PATH_MAX];
  getcwd(current_working_directory, sizeof(current_working_directory));

  DIR *directory = opendir(current_working_directory);
  if(directory == NULL)
  {
    write(1, "Error!: Error showing current directory\n", sizeof("Error!: Error showing current directory\n"));
    return;
  }

  struct dirent *dentry = readdir(directory);
  do {
    char *filename = dentry->d_name;
    write(1, filename, sizeof(char)*strlen(filename));
    write(1, " ", sizeof(char)*sizeof(" "));
  } while((dentry = readdir(directory)));
  write(1, "\n", sizeof(char)*sizeof("\n"));
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
    write(1, "Error!: Error showing current directory\n", sizeof(char)*strlen("Error!: Error showing current directory\n"));
    return;
  }

  write(1, current_working_directory, sizeof(char)*strlen(current_working_directory));
  write(1, "\n", sizeof(char)*sizeof("\n"));
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
    write(1, "Error!: Error showing current directory\n", sizeof(char)*strlen("Error!: Error showing current directory\n"));
    return;
  }

  // open the directory
  DIR *directory = opendir(current_working_directory);
  if (!directory) {
    write(1, "Error!: Cannot open directory\n", sizeof(char)*strlen("Error!: Cannot open directory\n"));
  }

  // read the directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(dirName, dentry->d_name) == 0)
    {
      write(1, "Error!: Directory already exists\n", sizeof(char)*strlen("Error!: Directory already exists\n"));

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
    write(1, "Error!: Unable to change to given directory\n", sizeof(char)*strlen("Error!: Unable to change to given directory\n"));
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
    write(1, "Error!: Error showing current directory\n", sizeof(char)*strlen("Error!: Error showing current directory\n"));
    return;
  }

  // open the directory
  DIR *directory = opendir(current_working_directory);
  if (!directory) {
    write(1, "Error!: Cannot open directory\n", sizeof(char)*strlen("Error!: Cannot open directory\n"));
    return;
  }

  // read the Directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(destinationPath, dentry->d_name) == 0)
    {
      write(1, "Error!: Given File already exists\n", sizeof(char)*strlen("Error!: Given File already exists\n"));

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
    write(1, "Error!: Issue opening source file\n", sizeof(char)*strlen("Error!: Issue opening source file\n"));
    close(destination_path);
    close(source_path);
    return;
  }

  if (destination_path == -1)
  {
    write(1, "Error!: Issue opening destination file\n", sizeof(char)*strlen("Error!: Issue opening destination file\n"));
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
    write(1, "Error!: Error showing current directory\n", sizeof(char)*strlen("Error!: Error showing current directory\n"));
    return;
  }

  // open the directory
  DIR *directory = opendir(current_working_directory);
  if (!directory) {
    write(1, "Error!: Cannot open directory\n", sizeof(char)*strlen("Error!: Cannot open directory\n"));
  }

  // read the Directory
  struct dirent *dentry = readdir(directory);
  // check if the directory already exists
  do {

    // if the directory already exists
    if(strcmp(destinationPath, dentry->d_name) == 0)
    {
      write(1, "Error!: Given Directory already exists\n", sizeof(char)*strlen("Error!: Given Directory already exists\n"));

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
    write(1, "Error!: Issue opening source file\n", sizeof(char)*strlen("Error!: Issue opening source file\n"));
    close(destination_path);
    return;
  }

  if (destination_path == -1)
  {
    write(1, "Error!: Issue opening destination file\n", sizeof(char)*strlen("Error!: Issue opening destination file\n"));
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
    write(1, "Error!: Error showing current directory\n", sizeof(char)*strlen("Error!: Error showing current directory\n"));
    return;
  }

  // open the directory
  DIR *directory = opendir(current_working_directory);
  if (!directory) {
    write(1, "Error!: Cannot open directory\n", sizeof(char)*strlen("Error!: Cannot open directory\n"));
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
        write(1, "Error!: Cannot delete given file\n", sizeof(char)*strlen("Error!: Cannot delete given file\n"));
        return;
      }

      // close the Directory
      closedir(directory);
      return;
    }
  } while((dentry = readdir(directory)));

  closedir(directory); // close the directory

  write(1, "Error!: No such file exists\n", sizeof(char)*strlen("Error!: No such file exists\n"));
  return;

}

void displayFile(char *filename)
{
  int file_path = open(filename, O_RDONLY);
  if (file_path == -1)
  {
    write(1, "Error!: Issue opening file\n", sizeof(char)*strlen("Error!: Issue opening file\n"));
    return;
  }

  char buf[PATH_MAX];
  int char_count = read(file_path, buf, PATH_MAX);
  do {
    write(1, buf, char_count);
  } while((char_count = read(file_path, buf, PATH_MAX)) > 0);

  close(file_path);
  return;
}
