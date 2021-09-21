//https://stackoverflow.com/questions/22886290/c-get-all-files-with-certain-extension
//https://stackoverflow.com/questions/5297248/how-to-compare-last-n-characters-of-a-string-to-another-string-in-c
//

//compile with: gcc -o uid_fixer uid_fixer.c $(xml2-config --cflags --libs)


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <libxml/xpath.h>
#include <libxml/parser.h>


//global variable
const char name_prefix[100] = "xx-xxx-xxxx-XXX_";

//these scan all subdirectories in the path directory and all files in each subdirectory
int directory_scan(char *path);
int directory_contents_scan(char *path, char *dir_name_string);

//ao filename needs to be changed, so ao has a separate file handler process
int ao_file_handler(char *target_dir, char *ao_filename, char *dir_name_string);

//this one formats filenames (slash vs. dash)
char* format_strings(char *target_string);

//xpath function set
int xpath_file_handler(char *target_dir, char *filename, char *dir_name_string, char *file_xpath);
xmlDocPtr getdoc (char *docname);
xmlXPathObjectPtr getnodeset (xmlDocPtr doc, xmlChar *xpath);
void save_to_file(xmlDoc *doc, char *save_file_name);


int
main(int argc, char **argv)
{


  char path[60] = "..//STAGING";

  /*
  if (argc < 2)
  {
    printf("ERR: usage is: %s desired_file_path\n", argv[0]);
    return -1;
  }

  strcpy(path, argv[1]);
  */

  directory_scan(path);

  return 0;
}


int
directory_scan(char *path)
{

  struct dirent **directory_namelist;
  int dn;

  char dir_name_string[100];

  dn = scandir(path, &directory_namelist, NULL, alphasort);

  if (dn < 0)
    perror("scandir");
  else {
    while (dn--){
      strncpy(dir_name_string, directory_namelist[dn]->d_name, 49);

      directory_contents_scan(path, dir_name_string);
      free(directory_namelist[dn]);
    }
    free(directory_namelist);
  }
  return 0;
}


int 
directory_contents_scan(char *path, char *dir_name_string)
{

  struct dirent **dir_contents_namelist;
  int dcn;
  char target_dir[200];

  snprintf(target_dir, sizeof target_dir, "%s/%s", path, dir_name_string);

  dcn = scandir(target_dir, &dir_contents_namelist, NULL, alphasort);

  /*
   * just for testing..i forget what these are sometimes
   printf("path %s\n", path);
   printf("target_dir %s\n", target_dir);
   printf("dir_name_string %s\n", dir_name_string);
   */


  if (dcn < 0)
    //NOTE: (fixed the root cause in the zipper program, but FYI) 
    //this will actually print if you do not remove ..zip from STAGING prior to the next run
    //of uid_fixer because a .zip is a file (not a directory)
    //but because it is ..zip it will be hidden in file browser unless you set to "show hidden files"
    perror("scandir");
    //printf("\n");
  else {

    while (dcn--){

      if(dir_contents_namelist[dcn]->d_type == DT_REG){

        const char *ext = strrchr(dir_contents_namelist[dcn]->d_name, '.');
        if((!ext) || (ext == dir_contents_namelist[dcn]->d_name))
          return 0;
        else {


          if(strcmp(ext, ".ncx") == 0 ){

            char ncx_filename[100];
            snprintf(ncx_filename, sizeof ncx_filename, "%s", dir_contents_namelist[dcn]->d_name);

            char ncx_xpath[100] = "//head/meta[1]/@content";          

            xpath_file_handler(target_dir, ncx_filename, dir_name_string, ncx_xpath);

          }

          if(strcmp(ext, ".pncx") == 0){

            char pncx_filename[100];
            snprintf(pncx_filename, sizeof pncx_filename, "%s", dir_contents_namelist[dcn]->d_name);

            char pncx_xpath[100] = "//head/meta[1]/@content";

            xpath_file_handler(target_dir, pncx_filename, dir_name_string, pncx_xpath);

          }


          if(strcmp(ext, ".ao") == 0){

            char ao_filename[200];
            snprintf(ao_filename, sizeof ao_filename, "%s", dir_contents_namelist[dcn]->d_name);
            ao_file_handler(target_dir, ao_filename, dir_name_string);

          }

          if(strcmp(ext, ".smil") == 0){

            char pdtb_smil[10] = "pdtb";

            //NOTE: do not need pdtb_protected.smil, so this filters it out
            if (strncmp(dir_contents_namelist[dcn]->d_name, pdtb_smil, 4) != 0){
                    
              
              
              char smil_filename[100];
              snprintf(smil_filename, sizeof smil_filename, "%s", dir_contents_namelist[dcn]->d_name);

              char smil_xpath[100] = "//head/meta[1]/@content";

              xpath_file_handler(target_dir, smil_filename, dir_name_string, smil_xpath);
              
            }
          }

          if(strcmp(ext, ".opf") == 0){
      
            char opf_filename[100];
            snprintf(opf_filename, sizeof opf_filename, "%s", dir_contents_namelist[dcn]->d_name);


           
            char opf_xpath[200] = "//*[local-name()='Identifier'] | //*[local-name()='metadata']//*[local-name()='x-metadata']//*[local-name()='meta'][last()]/@content";


            xpath_file_handler(target_dir, opf_filename, dir_name_string, opf_xpath);
            
          }

          if(strcmp(ext, ".ppf") == 0){
          
            char ppf_filename[100];
            snprintf(ppf_filename, sizeof ppf_filename, "%s", dir_contents_namelist[dcn]->d_name);

            char ppf_xpath[200] = "//*[local-name()='Identifier'] | //*[local-name()='manifest']//*[local-name()='item'][3]/@href";

            xpath_file_handler(target_dir, ppf_filename, dir_name_string, ppf_xpath);

          }
        }
      }
      free(dir_contents_namelist[dcn]);
    }
    free(dir_contents_namelist);
  }

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
ao_file_handler(char *target_dir, char *ao_filename, char *dir_name_string)
{

  char handoff_dir_name[300];

  snprintf(handoff_dir_name, sizeof handoff_dir_name, "%s", dir_name_string);
  
  format_strings(handoff_dir_name);

  char old_ao_filename[300];
  char new_ao_filename[300];

  snprintf(old_ao_filename, sizeof old_ao_filename, "%s/%s", target_dir, ao_filename);
  snprintf(new_ao_filename, sizeof new_ao_filename, "%s/%s%s.ao", target_dir, name_prefix, handoff_dir_name); 

  rename(old_ao_filename, new_ao_filename);

  char handoff_ao_filename[300];

  snprintf(handoff_ao_filename, sizeof handoff_ao_filename, "%s%s.ao", name_prefix, handoff_dir_name);

  char ao_xpath[300] = "//*[local-name()='Book']/@uid | //*[local-name()='uid']";


  xpath_file_handler(target_dir, handoff_ao_filename, dir_name_string, ao_xpath);


  return 0;

}



int 
xpath_file_handler(char *target_dir, char *filename, char* dir_name_string, char *file_xpath)
{

  char *docname;
  xmlDocPtr doc;

  xmlNodePtr pNode = NULL;
  xmlNodeSetPtr pNodeSet = NULL;



  xmlChar *xpath;


  xmlChar xpath_string[300];
  snprintf(xpath_string, sizeof xpath_string, "%s", file_xpath);

  xpath = (xmlChar*) xpath_string;
  


  xmlNodeSetPtr nodeset;
  xmlXPathObjectPtr result;
  int i;
  xmlChar *keyword;


  char aoext [5] = ".ao";


  char format_dir_name_string[100];
  snprintf(format_dir_name_string, sizeof format_dir_name_string, "%s", dir_name_string);

  //format slashes/dashes
  format_strings(format_dir_name_string);

  //create the complete uid
  char uid_string[300];
  snprintf(uid_string, sizeof uid_string, "%s%s", name_prefix,format_dir_name_string);

  //create completed .ao filename
  char uid_string_ext_ao[300];
  snprintf(uid_string_ext_ao, sizeof uid_string_ext_ao, "%s%s.ao", name_prefix,format_dir_name_string);


  char temp_file_name[300];
  snprintf(temp_file_name, sizeof temp_file_name, "%s.temp", filename);


  char docname_string[300];
  snprintf(docname_string, sizeof docname_string, "%s/%s", target_dir, filename);

  docname = docname_string;

  doc = getdoc(docname);
  result = getnodeset (doc, xpath);



  if(result){

    nodeset = result->nodesetval;
    for (i = 0; i < nodeset->nodeNr; i++) {


 
      keyword = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);

      pNode = nodeset->nodeTab[i];
     
      int len = strlen(keyword);

      const char *last_three = &keyword[len-3];

      if (strcmp(last_three, aoext) == 0){

         xmlNodeSetContent(pNode, uid_string_ext_ao);

      }

      if (strcmp(last_three, aoext) !=0){


        xmlNodeSetContent(pNode, uid_string);

      }


      //NOTE: this printf() is only for testing, and will always display
      //the filenames, etc. PRIOR to change (the program will change them
      //though)
      //have to run program TWICE to see effects (updated filenames, etc.)
      //printf("[%d] %s\n    ->%s\n\n", i,filename,keyword);      
      



      
      save_to_file(doc, temp_file_name);


      xmlFree(keyword);


    }

    xmlXPathFreeObject (result);
  }
  xmlFreeDoc(doc);
  xmlCleanupParser();


  

  //remove original document
  remove(docname);

  //this renames the temp document 
  //to the name of the deleted (original) file
  rename(temp_file_name, docname);



  /*
     printf("%s\n", target_dir);
     printf("%s\n", filename);
     printf("%s\n", dir_name_string);
     printf("%s\n", xpath);
     */


  return 1;
}

xmlDocPtr
getdoc (char *docname)
{

  xmlDocPtr doc;
  doc = xmlParseFile(docname);

  if (doc == NULL){

    fprintf(stderr, "ERR: document not parsed successfully. \n");
    return NULL;
  }

  return doc;

}

xmlXPathObjectPtr
getnodeset (xmlDocPtr doc, xmlChar *xpath)
{

  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;

  context = xmlXPathNewContext(doc);
  if (context == NULL) {

    printf("ERR in xmlPathNewContext\n");
    return NULL;
  }

  result = xmlXPathEvalExpression(xpath, context);
  xmlXPathFreeContext(context);

  if (result == NULL) {

    printf("ERR in xmlXPathEvalExpression\n");
    return NULL;
  }

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)){

    xmlXPathFreeObject(result);
    printf("No result\n");

    return NULL;
  }

  return result;



}

void 
save_to_file(xmlDoc *doc, char *save_file_name)
{

  xmlChar *xmlbuff;
  int buffersize;
  FILE *fp;

  xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);

  fp = fopen(save_file_name, "w+");
  fputs(xmlbuff, fp);
  fclose(fp);

}





