#include "mysqlbase.h"

int	con_database(struct mysqlStruct *s,char *dataBase, char *ip){
    // 初始化MySQL
    s->db = mysql_init(NULL);
    if (s->db == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return -1;
    }


    // 连接到数据库
    if (mysql_real_connect(s->db, ip, "root", "", dataBase, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(s->db);
        return -2;
    }
}

int exec_mysql(struct mysqlStruct *s, char *sql, void* data){
    // 执行sql
    if (mysql_query(s->db, sql)) {
        fprintf(stderr, "%s: %s\n", sql, mysql_error(s->db));
        mysql_close(s->db);
        return -1;
    }

    // 获取结果集
    s->res = mysql_store_result(s->db);
    if (s->res == NULL) {
        fprintf(stderr, "mysql_store_result() failed. Error: %s\n", mysql_error(s->db));
        mysql_close(s->db);
        return -2;
    }

    struct mysql_res_struct *results = (struct mysql_res_struct *)data;
    MYSQL_FIELD *fields = mysql_fetch_fields(s->res);
    unsigned int num_fields = mysql_num_fields(s->res);
    MYSQL_ROW row;

    // 打印结果
    while ((row = mysql_fetch_row(s->res))) {
        // printf("Row: %s, %s, %s\n", row[0], row[1], row[2]); // 根据你的表结构修改索引

        results->rows = realloc(results->rows, (results->row_count+1)*sizeof(char*));
	    if(!results->rows){
			fprintf(stderr, "Memory allocation failed for rows\n");
			free_res(results);mysql_free_result(s->res);
			return -3;
	    }

        int strsize = 0;
        for(int i=0;i<num_fields;i++){
            strsize += strlen(fields[i].name) + (row[i]?strlen(row[i]):4) + 2;
        }
        results->rows[results->row_count] = malloc((strsize+2) * sizeof(char));
        memset(results->rows[results->row_count],0,strsize+2);
        if(!results->rows[results->row_count]){
            fprintf(stderr, "Memory allocation failed for columns in results->rows[%d]\n", results->row_count);
            free_res(results);mysql_free_result(s->res);
            return -4;
        }
	for(int i=0;i<num_fields;i++){
        // 格式化输出
        snprintf(results->rows[results->row_count] + (strlen(results->rows[results->row_count])?strlen(results->rows[results->row_count]):0 ),strsize + 2 - strlen(results->rows[results->row_count]),",%s=%s", fields[i].name, row[i] ? row[i] : "NULL");  
	}
        results->row_count += 1;
    }
    // 释放结果集
    mysql_free_result(s->res);
    return 0;
}


void closedb(struct mysqlStruct *s){
    // 关闭连接
    mysql_close(s->db);
}

//释放select的结果指针
int free_res(struct mysql_res_struct *res){
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


char *findValuebyKey(struct mysql_res_struct *res,char* key, int index){
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
 
void mysqlInit(struct mysqlStruct *structure){
	structure->connectDb	= con_database;
	structure->closeDb 	= closedb;
	structure->execMysql	= exec_mysql;
	structure->freeRes	= free_res;
}

int mysql_res_to_valArry(struct mysql_res_struct *res, char ***data, int index){
if(res == NULL || res->row_count<=index){
		//fprintf(stderr, "struct sqlite_res_struct res is NULL\n");
		return 0;
	}
	int arrySize = 0;
	*data = NULL;	

	char *indexLine = malloc(sizeof(char)*(strlen(res->rows[index])+5));
	strcpy(indexLine, res->rows[index]);

	char *token = strtok(indexLine,",");
	char *tokenTemp = strtok(NULL,"");
	int tokenlen = 0;
	while(token){
	//	printf("-- %s\n",token);
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

void freeValArray(char **data, int dataSize){
    if(data){
		while(dataSize>0){
			dataSize--;
			free(data[dataSize]);
		}
		free(data);	
	}
}

