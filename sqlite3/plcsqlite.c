#include "plcsqlite.h"


//创建并初始化struct sqliteStruct结构
struct sqliteStruct* plcSqlite_init(){
	struct sqliteStruct *sqliteStructure = (struct sqliteStruct*)malloc(sizeof(struct sqliteStruct));
	if(sqliteStructure == NULL){
			return NULL;
	}
	sqliteInit(sqliteStructure);
	
	return sqliteStructure;
}

//关闭数据库连接并  释放struct sqliteStruct   也可以释放struct sqlite_res_struct结构体
void plcSqlite_free(struct sqliteStruct* sqliteStructure, struct sqlite_res_struct *selectRes){
	
	if(selectRes != NULL){
		sqliteStructure->freeRes(selectRes);
	}
	
	if(sqliteStructure != NULL){
		sqliteStructure->closeSqlite(sqliteStructure);
		free(sqliteStructure);
	}
	sqliteStructure = NULL;
}

//修改表名
int updateTableName(struct sqliteStruct* sqliteStructure, char* oldName, char* newName){
	char sql[100];
	snprintf(sql,100,"ALTER TABLE %s RENAME TO %s;",oldName,newName);
	return sqliteStructure->execSqlite(sqliteStructure,sql,NULL);
}
//创建表
int createTable(struct sqliteStruct* sqliteStructure, char* name){

	char sql[200];
	snprintf(sql,200,"CREATE TABLE %s (id INTEGER PRIMARY KEY AUTOINCREMENT,value TEXT,timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);",name);
	return sqliteStructure->execSqlite(sqliteStructure,sql,NULL);	
}
//删除表
int deleteTable(struct sqliteStruct* sqliteStructure, char* name){
	char sql[100];
	snprintf(sql,100,"DROP TABLE %s;",name);
	return sqliteStructure->execSqlite(sqliteStructure,sql,NULL);	
}

//删除表内所有内容，并充值自增字段
int delete_all_data(struct sqliteStruct* sqliteStructure, char* name){
	char sql[100];
	//删除表的所有数据
	snprintf(sql,100,"DELETE FROM %s;",name);
	if(sqliteStructure->execSqlite(sqliteStructure,sql,NULL)){
		return -1;
	}
	//重置自增字段
	snprintf(sql,100,"DELETE FROM sqlite_sequence WHERE name='%s';",name);
	if(sqliteStructure->execSqlite(sqliteStructure,sql,NULL)){
		return -2;
	}
	
	return 0;
}


//返回最近时间点的一条数据
//data[0]=value1
//data[1]=value2
//data[2]=value3
int select_last_data(struct sqliteStruct* sqliteStructure,char **resArray,char *tableName){
	char sql[100];
	struct sqlite_res_struct data = {NULL,0};
	snprintf(sql,100,"SELECT value, timestamp FROM %s ORDER BY timestamp DESC LIMIT 1;",tableName);
	if(sqliteStructure->execSqlite(sqliteStructure,sql,&data))
	{
		return -1;	
	}
	
	int resArraySize = sqlite_res_to_valArry(&data, &resArray, 0);
	sqliteStructure->freeRes(&data);
	return resArraySize;
}

//返回符合条件的数据个数
int isIn_tableNames(struct sqliteStruct* sqliteStructure, char *tableName){
	char sql[100];
	struct sqlite_res_struct data = {NULL,0};
	snprintf(sql,100,"SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",tableName);
	if(sqliteStructure->execSqlite(sqliteStructure,sql,&data)){
		return -1;	
	}
	int res = data.row_count;
	sqliteStructure->freeRes(&data);
	return res;
}



/*
 * ALTER TABLE old_table_name RENAME TO new_table_name;
 * 
 * SELECT * FROM data_table
	ORDER BY timestamp ASC;
	ASC 表示按升序（从旧到新）排序。
	DESC 表示按降序（从新到旧）排序，如果你想要按最新的时间排序，可以使用 DESC。
 * 
 * 
 * */







