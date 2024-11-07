#ifndef _SQLITE_H_
#define _SQLITE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdint.h>
struct sqliteStruct;
struct sqlite_res_struct;
int		con_sqlite(struct sqliteStruct *s,char *dataBase);
void	close_sqlite(struct sqliteStruct *s);
int 	exec_sqlite(struct sqliteStruct *s, char *sql_select,struct sqlite_res_struct *data);
int		free_res(struct sqlite_res_struct *res);
void	sqliteInit(struct sqliteStruct *structure);
int 	sqlite_res_to_valArry(struct sqlite_res_struct *res, char ***data, int index);
void 	freeValArray(char **data, int dataSize);
int		exec_atomic_sqlite(struct sqliteStruct *s, char *sql);
char*	findValuebyKey(struct sqlite_res_struct *res,char* key,int index);//返回的结果需要free
struct sqlite_res_struct {
	char **rows;
	int row_count;
};


struct sqliteStruct{
	sqlite3 *db;
	int  (*connectSqlite)(struct sqliteStruct *s,char *dataBase);
	void (*closeSqlite)(struct sqliteStruct *s);
	int  (*execSqlite)(struct sqliteStruct *s, char *sql_select,struct sqlite_res_struct *data);
	int	 (*execAtomicSqlite)(struct sqliteStruct *s, char *sql);
	int  (*freeRes)(struct sqlite_res_struct* res);
};

//struct sqliteStruct sqliteStructure;

#endif


