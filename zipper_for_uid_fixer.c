/* NOTES
 * not the greatest way to do this, but i need to zip all dirs after the UID 
 * has been fixed, and this was the quickest way to do it
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

//char name_prefix[20] = "xx-xxx-";
//char lib_name[20] = "xxxx-";

// compile with gcc dir_name_remove_space_zip.c -o dir_name_remove_space_zip


int dir_name_change(char *path);
char* format_strings(char *target_string);
int zip_all_files(char *path);

int
main()
{

  char path[50] = "..//_mag//STAGING//";

  dir_name_change(path);
  zip_all_files(path);

  return 0;
}


int
dir_name_change(char *path)
{
  struct dirent **namelist;
  int n;
  char dir_name_string[50];
  char original_dir_name[50];
  char final_dir_name[100];
  char useless_dir_name[3] = ".";
  int compare_filter;


  n = scandir(path, &namelist, NULL, alphasort);

  if (n < 0)
    perror("scandir");
  else {

    while (n--) {


      //strncpy(dir_name_string, namelist[n]->d_name, 8);

      //dir_name_string [8] = '\0';

      strcpy(dir_name_string, namelist[n]->d_name);

      //NOTE: dont need the extra conditional
      //      i thought ..zip was created  at this phase
      //      but i think it is happening due to syntax of execl()
      //      so just added an extra command to remove useless ..zip file
      //
      //
      //compare_filter = strcmp(dir_name_string, useless_dir_name);

      //if (compare_filter == 0)
      //{
        //printf("useless dir name filtered out");
      //}

      //else
      //{
      

        //char *path_name_prefix;
        //path_name_prefix = strcat(path, name_prefix);

        snprintf(final_dir_name, sizeof final_dir_name, "%s%s", path, dir_name_string);

        snprintf(original_dir_name, sizeof original_dir_name, "%s%s", path, dir_name_string);


        format_strings(final_dir_name);

        //printf("%s\n", final_dir_name);


      
        rename(original_dir_name,final_dir_name);
      

      free(namelist[n]);

    }//end of while loop

    free(namelist);
  }//end of conditional

  return 0;
}


char*
format_strings(char *target_string)
{

  char *ptr = target_string;
    
  while (*ptr)
    {
      if((*ptr == '+') || (*ptr == '-'))
        *ptr = '_';
      if(*ptr == ' ')
        *ptr = '-';
      ptr++;
   }

  return target_string;

}

int 
zip_all_files(char *path)
{

  char binary_path[400];
  char arg_1 [5];
  char term_command_1[5];
  char term_command_2[300];
  char term_command_3[50];

  char final_command_string[500];

  strcpy(binary_path, "/bin/bash");
  strcpy(arg_1, "-c");
  strcpy(term_command_1, "cd");
  strcpy(term_command_2, "find . -type d -exec zip -r {}.zip {} \\;");
  strcpy(term_command_3, "rm ..zip");

  snprintf(final_command_string, sizeof final_command_string, "%s %s && %s && %s", term_command_1, path, term_command_2, term_command_3);


  execl(binary_path, binary_path, arg_1, final_command_string, NULL);


  return 0;

}


