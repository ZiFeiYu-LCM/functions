#ifndef _MYSQLBASE_H
#define _MYSQLBASE_H
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>

struct mysqlStruct;
struct mysql_res_struct;
int	con_database(struct mysqlStruct *s,char *dataBase,char *ip);
int     exec_mysql(struct mysqlStruct *s, char *sql, void* data);
void    closedb(struct mysqlStruct *s);
int     free_res(struct mysql_res_struct *res);
char*   findValuebyKey(struct mysql_res_struct *res,char* key, int index);//返回的结果需要free
void	mysqlInit(struct mysqlStruct *structure);
int 	mysql_res_to_valArry(struct mysql_res_struct *res, char ***data, int index);
void 	freeValArray(char **data, int dataSize);

struct mysql_res_struct {
	char **rows;
	int row_count;
};


struct mysqlStruct{
	MYSQL *db;
    MYSQL_RES *res;
    
	int  (*connectDb)(struct mysqlStruct *s,char *dataBase,char *ip);
	void (*closeDb)(struct mysqlStruct *s);
	int  (*execMysql)(struct mysqlStruct *s, char *sql,void *data);
	int  (*freeRes)(struct mysql_res_struct* res);
};



#endif