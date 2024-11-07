#include "sqlite.h"
#include <string.h>
#include <stdlib.h>
//select回调函数， 注意没有查到对应的匹配内容的时候，是不会调用select_callback函数的
static int select_callback(void *data, int argc, char **argv, char **colName){
	
	struct sqlite_res_struct *results = (struct sqlite_res_struct *)data;
	
	results->rows = realloc(results->rows, (results->row_count+1)*sizeof(char*));
	if(!results->rows){
			fprintf(stderr, "Memory allocation failed for rows\n");
			free_res(results);
			return 1;
	}
	
	int strsize = 0;
	for(int i=0;i<argc;i++){
		strsize += strlen(colName[i]) + ( argv[i]?strlen(argv[i]):4 ) + 2;
	}
	results->rows[results->row_count] = malloc((strsize+2) * sizeof(char));
	memset(results->rows[results->row_count],0,strsize+2);
	if(!results->rows[results->row_count]){
		fprintf(stderr, "Memory allocation failed for columns in results->rows[%d]\n", results->row_count);
		free_res(results);
		return 1;
	}
	//返回形式为  %s=%s,
	for(int i=0;i<argc;i++){
		//第一个地方多出来一个，    这个都好在findValuebyKey中配置 ,key= 来选取正确的key
		//printf("--test: %s\n", results->rows[results->row_count]);
		snprintf(results->rows[results->row_count] + (strlen(results->rows[results->row_count])?strlen(results->rows[results->row_count]):0 ),strsize + 2 - strlen(results->rows[results->row_count]),",%s=%s", colName[i], argv[i] ? argv[i] : "NULL");  // 格式化输出
	}
	//消除最后一个多余的，
	// results->rows[results->row_count][strsize-1] = '\0';
	printf("row_count : %d\n",results->row_count);
	results->row_count += 1;
	return 0;
}

//连接
int con_sqlite(struct sqliteStruct *s, char *dataBase){

	int rc;
	rc = sqlite3_open(dataBase,&(s->db));
	if(rc!=SQLITE_OK){
		fprintf(stderr,"cannot open database:%s\n",sqlite3_errmsg(s->db));
		return rc;
	}
	printf("连接数据库 成功\n");
	return 0;
}


//关闭连接
void close_sqlite(struct sqliteStruct *s){
	if(s->db){
		sqlite3_close(s->db);
	}
}


//调用sqlite的执行函数, 增加了原子操作， 一般删除和插入需要
int exec_atomic_sqlite(struct sqliteStruct *s, char *sql){
	if(s->db == NULL){
		fprintf(stderr,"cannot open database:%s\n", "db is NULL");
		return 1;
	}
	
	char *err_msg = 0;
    // 开始事务
    if (sqlite3_exec(s->db, "BEGIN TRANSACTION;", 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Can't begin transaction: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }
	
	//执行语句
	if (sqlite3_exec(s->db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(s->db, "ROLLBACK;", 0, 0, &err_msg); // 出现错误时回滚
        return 2;
    }
	
	// 提交事务
    if (sqlite3_exec(s->db, "COMMIT;", 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Can't commit transaction: %s\n", err_msg);
        sqlite3_free(err_msg);
		return 3;
    }
	
	return 0;
}



//调用sqlite的执行函数,  如果执行select函数，则data不能为NULL，   如果执行select意外的函数data为NULL
int exec_sqlite(struct sqliteStruct *s, char *sql_select,struct sqlite_res_struct *data){
	if(s->db == NULL){
		fprintf(stderr,"cannot open database:%s\n", "db is NULL");
		return 1;
	}
	
	char *err_msg = 0;
	int rc;
	if(data){
		rc = sqlite3_exec(s->db,sql_select, select_callback, (void*)data, &err_msg);
	}else{
		rc = sqlite3_exec(s->db,sql_select, 0, 0, &err_msg);
	}
	if(rc!=SQLITE_OK){
		fprintf(stderr,"Failed to sqlite3_exec sql:%s\n",err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(s->db);
		if(data){
			free_res(data);
		}
		return rc;
	}
	return 0;
}

//释放select的结果指针
int free_res(struct sqlite_res_struct *res){
	if(res == NULL)
		return 0;
	
	for(int i=0;i<res->row_count;i++){
			if(res->rows[i])
				free(res->rows[i]);
	}
	
	if(res->rows)
		free(res->rows);

	res->row_count = 0;
	return 0;
}


//释放结果
void sqliteInit(struct sqliteStruct *structure){
	structure->connectSqlite	= con_sqlite;
	structure->closeSqlite 		= close_sqlite;
	structure->execSqlite		= exec_sqlite;
	structure->freeRes			= free_res;
	structure->execAtomicSqlite = exec_atomic_sqlite;
}

/*
将对应index的数据从  key1=value1,key2=value2,key3=value3,
转化为 一个char的数组，每个元素为value的值
data[0]=value1
data[1]=value2
data[2]=value3
*/
int sqlite_res_to_valArry(struct sqlite_res_struct *res, char ***data, int index){
	if(res == NULL || res->row_count<=index){
		//fprintf(stderr, "struct sqlite_res_struct res is NULL\n");
		return 0;
	}
	int arrySize = 0;
	

	char *indexLine = malloc(sizeof(char)*(strlen(res->rows[index])+5));
	strcpy(indexLine, res->rows[index]);

	char *token = strtok(indexLine,",");
	//char *token = strtok(NULL,",");
	char *tokenTemp = strtok(NULL,"");
	int tokenlen = 0;
	while(token){
//		printf("-- %s\n",token);
		tokenlen = strlen(token);
		(*data) = realloc(*data, sizeof(char*) * (arrySize+2));
		if (!(*data)) {
            fprintf(stderr, "Memory allocation failed for data array\n");
            return -1;
        }
		(*data)[arrySize] = malloc(sizeof(char) * (tokenlen+2));
		memset((*data)[arrySize],0,(tokenlen+1));
        if (!(*data)[arrySize]) {
            fprintf(stderr, "Memory allocation failed for token string\n");
            return -1;
        }
		//char *key 	= strtok(token,"=");
		strtok(token,"=");
		char *value = strtok(NULL,"=");
		// snprintf((*data)[arrySize],tokenlen,"%s",value);
		strncpy((*data)[arrySize],value,tokenlen);
		arrySize += 1;
		
		token = strtok(tokenTemp,",");
		tokenTemp = strtok(NULL,"");
	}	
	free(indexLine);
	return arrySize;
}

char *findValuebyKey(struct sqlite_res_struct *res,char* key, int index){
	if(res == NULL || res->row_count<=index){
		return NULL;
	}
	char *indexLine = res->rows[index];
	
	char *str = (char*)malloc(strlen(indexLine)+5);
	strcpy(str,indexLine);

	char *_key = (char*)malloc(strlen(key)+5);
	sprintf(_key,",%s=",key);

	char *str2 = strstr(str,_key);
	strtok(str2,",");
	strtok(NULL,",");
	strtok(str2,"=");
	char *tempStr = strtok(NULL,"=");
	char *resStr = (char*)malloc((1+strlen(tempStr))*sizeof(char));
	strcpy(resStr,tempStr);
	free(str);
	free(_key);
	return resStr;
}


void freeValArray(char **data, int dataSize){
	if(data){
		while(dataSize>0){
			dataSize--;
			free(data[dataSize]);
		}
		free(data);	
	}
}




/*
int main(){
	struct sqliteStruct sqliteStructure;
	sqliteInit(&sqliteStructure);
	
	struct sqlite_res_struct selectRes = {NULL,0};
	char *sql_select = "SELECT * FROM users;";
	if(sqliteStructure.connectSqlite(&sqliteStructure, "/sqlite/routerbase.db")){
			return 1;
	}

	sqliteStructure.selectSqlite(&sqliteStructure,sql_select,&selectRes);


	//printf("voer \n");
	for(int i=0;i<selectRes.row_count;i++){
		printf("%s\n",selectRes.rows[i]);
	}


	char **resArry = NULL;
	int arrySize = sqlite_res_to_valArry(&selectRes,&resArry,0);
	for(int i=0;i<arrySize;i++){
			printf("%s\n",  resArry[i]);
		
	}
	freeValArray(resArry, arrySize);


	
	sqliteStructure.freeRes(&selectRes);
	sqliteStructure.closeSqlite(&sqliteStructure);
	return 0;
}
*/


