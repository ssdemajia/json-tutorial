#ifndef LEPTJSON_H__
#define LEPTJSON_H__

typedef enum {
     LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT 
}lept_type;

typedef struct {
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,//错误代码，当json中只含有空白
    LEPT_PARSE_INVALID_VALUE,//在两个值中间还有空白
    LEPT_PARSE_ROOT_NOT_SINGULAR//不是null、false、true之一
};

int lept_parse(lept_value* v, const char* json);//解析json， v是根节点，也相当于入口
//返回值是上面的LEPT_PARSE_OK等
lept_type lept_get_type(const lept_value* v);//得到这个节点的类型，比如LEPT_NULL,LEPT_FALSE等


#endif /* LEPTJSON_H__ */
/* 
JSON-text = ws value ws
ws        = *(%x20 / %x09 / %x0A / %x0D)
value     = null / false / true 
null      = "null" 字面值
false     = "false" 字面值
true      = "true" 自面子
 */