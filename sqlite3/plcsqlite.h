#ifndef _PLCSQLITE_H_
#include "../database/mysqlbase.h"




struct sqliteStruct* plcSqlite_init();

//第二个参数可以释放结果指针，也可以为NULL自己手动释放
void plcSqlite_free(struct sqliteStruct* sqliteStructure, struct sqlite_res_struct *selectRes);


int updateTableName(struct sqliteStruct* sqliteStructure, char* oldName, char* newName);
int createTable(struct sqliteStruct* sqliteStructure, char* name);
int deleteTable(struct sqliteStruct* sqliteStructure, char* name);
int select_last_data(struct sqliteStruct* sqliteStructure,char **resArray,char *tableName);
int isIn_tableNames(struct sqliteStruct* sqliteStructure, char *tableName);
int delete_all_data(struct sqliteStruct* sqliteStructure, char* name);



#endif
