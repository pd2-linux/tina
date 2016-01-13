#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsa_api.h"

extern BD_NAME local_device_conf_name;
extern BD_ADDR local_device_set_addr;

#define CONF_COMMENT '#'
#define CONF_DELIMITERS " =\n\r\t"
#define CONF_VALUES_DELIMITERS "\"=\n\r\t"
#define CONF_MAX_LINE_LEN 255

typedef int (conf_action_t)(char *p_conf_name, char *p_conf_value);

typedef struct {
    const char *conf_entry;
    conf_action_t *p_action;
} conf_entry_t;

int device_name_cfg(char *p_conf_name, char *p_conf_value);
int device_addr_set(char *p_conf_name, char *p_conf_value);

/*
 * Current supported entries and corresponding action functions
 */
static const conf_entry_t conf_table[] = {
    {"Name", device_name_cfg},
    {"BtAddr", device_addr_set},
    {(const char *) NULL, NULL}
};

/*****************************************************************************
**   FUNCTIONS
*****************************************************************************/
int str2bd(char *str, BD_ADDR addr)
{
    int i = 0;
    for (i = 0; i < 6; i++) {
       addr[i] = (UINT8) strtoul(str, (char **)&str, 16);
       str++;
    }
    return 0;
}

int device_name_cfg(char *p_conf_name, char *p_conf_value)
{
    strcpy((char *)local_device_conf_name, p_conf_value);
    return 0;
}

int device_addr_set(char *p_conf_name, char *p_conf_value)
{
	  int i = 0;
	  char *str = p_conf_value;
	  
	  if (!p_conf_value){
	      local_device_set_addr[0] = '\0';
	      return ;
	  }
	  
    for (i = 0; i < 6; i++) {
       local_device_set_addr[i] = (UINT8) strtoul(str, (char **)&str, 16);
       str++;
    }
    
    return 0;
}

/*******************************************************************************
**
** Function        bta_load_conf
**
** Description     Read conf entry from p_path file one by one and call
**                 the corresponding config function
**
** Returns         None
**
*******************************************************************************/
void bta_load_conf(const char *p_path)
{
    FILE    *p_file;
    char    *p_name;
    char    *p_value;
    conf_entry_t    *p_entry;
    char    line[CONF_MAX_LINE_LEN+1]; /* add 1 for \0 char */
    BOOLEAN name_matched;

    printf("Attempt to load stack conf from %s\n", p_path);

    if ((p_file = fopen(p_path, "r")) != NULL)
    {
        /* read line by line */
        while (fgets(line, CONF_MAX_LINE_LEN+1, p_file) != NULL)
        {
        	  printf("%s\n", line);
            if (line[0] == CONF_COMMENT)
                continue;

            p_name = strtok(line, CONF_DELIMITERS);
            
            if (NULL == p_name)
            {
                continue;
            }

            p_value = strtok(NULL, CONF_VALUES_DELIMITERS);

            if (NULL == p_value)
            {
                printf("bte_load_conf: missing value for name: %s\n", p_name);
                continue;
            }

            name_matched = FALSE;
            p_entry = (conf_entry_t *)conf_table;

            while (p_entry->conf_entry != NULL)
            {
                if (strcmp(p_entry->conf_entry, (const char *)p_name) == 0)
                {
                    name_matched = TRUE;
                    if (p_entry->p_action != NULL)
                        p_entry->p_action(p_name, p_value);
                    break;
                }

                p_entry++;
            }
        }

        fclose(p_file);
    }
    else
    {
        printf( "bte_load_conf file >%s< not found\n", p_path);
    }
}

void bta_load_addr(const char *p_path)
{
    FILE    *p_file;
    char    line[CONF_MAX_LINE_LEN+1]; /* add 1 for \0 char */
    
    printf("Attempt to read mac addr from %s\n", p_path);
    
    if ((p_file = fopen(p_path, "r")) != NULL){
        printf("open %s success!\n", p_path);
        if (fgets(line, CONF_MAX_LINE_LEN+1, p_file) != NULL){
        	  printf("line:%s\n", line);
            device_addr_set(NULL, line);
            return ;        
        } else {
            printf("read file error!\n");
        } 
    }
    
    printf("open file error!\n");
    
    device_addr_set(NULL, NULL);    	
}