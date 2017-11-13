#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <stdbool.h> /* bool false true */
#include <math.h> /* HUGE_VAL */
#include <errno.h> /* errno, ERANGE */
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)  ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)  ((ch) >= '1' && (ch) <= '9')
#define ISSPACE(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r' || (ch) == '\v' || (ch) == '\f')
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (ISSPACE(*p))
        p++;
    c->json = p;
}

// static int lept_parse_true(lept_context* c, lept_value* v) {
//     EXPECT(c, 't');
//     if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
//         return LEPT_PARSE_INVALID_VALUE;
//     c->json += 3;
//     v->type = LEPT_TRUE;
//     return LEPT_PARSE_OK;
// }

// static int lept_parse_false(lept_context* c, lept_value* v) {
//     EXPECT(c, 'f');
//     if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
//         return LEPT_PARSE_INVALID_VALUE;
//     c->json += 4;
//     v->type = LEPT_FALSE;
//     return LEPT_PARSE_OK;
// }

// static int lept_parse_null(lept_context* c, lept_value* v) {
//     EXPECT(c, 'n');
//     if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
//         return LEPT_PARSE_INVALID_VALUE;
//     c->json += 3;
//     v->type = LEPT_NULL;
//     return LEPT_PARSE_OK;
// }
static int lept_parse_literal(lept_context* c, const char* literal)
{
    EXPECT(c, literal[0]);
    size_t len = strlen(literal);
    for (size_t i = 1; i < len; i++) {        
        if (c->json[i-1] != literal[i]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += len-1;//因为在EXPECT中已经让c->json加1了，所以不需要再加len长度
    return LEPT_PARSE_OK;
}
static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    const char* str = c->json;
    size_t len = strlen(str);
    bool isNegative = false;//用于检查是否是负数
    bool isZeroStart = false;//用于检查是否小数点左边是0
    bool isDigitStart = false;//用于检查是否是数字开头
    bool isScience = false;//用于检查是否是科学技术法
    bool isPoint = false;//用于检查是否是小数
    for (int i = 0; i < len;) 
    {
        if (!isNegative && str[i] == '-') 
        {//开头是负数
            isNegative = true;
            i++;
        }
        else if (!isZeroStart && str[i] == '0') 
        {//前面是从0开始
            isZeroStart = true;
            i++;
            if (i != len && !(str[i]=='.' || str[i]=='e'||str[i]=='E')) return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
        else if (!isDigitStart && ISDIGIT1TO9(str[i])) 
        {//前半部分是从非零数1-9开始到后面是数字
            isDigitStart = true;
            i++;//第一位是1-9，检查后就跳过去
            while (ISDIGIT(str[i])) {//检查后面的数，这些数在小数点和科学计数法E(e)之前
                i++;
            }
        }
        else if (!isPoint && str[i] == '.' && (isZeroStart || isDigitStart))//必须前面是0或者是数字
        {//前半部分结束，开始判断后面的部分
            isPoint = true;
            i++;//跳过小数点
            if (i == len) return LEPT_PARSE_INVALID_VALUE;//小数点后面必须有数字
            while (ISDIGIT(str[i])) i++;//将小数点后面的数字跳过
        }
        else if (!isScience && (str[i] == 'e' || str[i] == 'E') && (isZeroStart || isDigitStart))
        {
            isScience = true;
            i++;//科学技术法前面必须是小数或者是整数
            if (str[i] == '+' || str[i] == '-' )  {
                i++;
            }
            while (ISDIGIT(str[i])) i++;
        }
        else return LEPT_PARSE_INVALID_VALUE; 
    }
    v->n = strtod(c->json, &end);
    // if (c->json == end)
    //     return LEPT_PARSE_INVALID_VALUE;
    
    c->json = end;
    if (errno == ERANGE && (v->n == HUGE_VAL || -v->n == HUGE_VAL)) {
        v->type = LEPT_NULL;
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    char first = *c->json;
    
    switch (first) {
        case 't':  
            v->type = LEPT_TRUE;
            return lept_parse_literal(c, "true");
        case 'f':  
            v->type = LEPT_FALSE;
            return lept_parse_literal(c, "false");
        case 'n':  
            v->type = LEPT_NULL;
            return lept_parse_literal(c, "null");
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
