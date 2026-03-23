#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sqlite3.h>
#include <libpq-fe.h>

#define ERROR_EXIT(fmt, label, ...) \
    do { \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        res = -1; \
        goto label; \
    } while (0)

#define ERROR_FN_EXIT(db, err_fn, fmt, label) \
    do { \
        fprintf(stderr, fmt ": %s\n", err_fn(db)); \
        res = -1; \
        goto label; \
    } while (0)

// Структура для хранения результатов статистики
typedef struct
{
    double avg;
    double max;
    double min;
    double var;
    double sum;
} DBStats;

// Получение данных для SQLite
int process_sqlite(const char *db_name, const char *table, const char *col, DBStats *stats)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *sql_res = NULL;
    int res = 0; // результат возвращаемый функцией

    if (sqlite3_open(db_name, &db) != SQLITE_OK)
    {
        ERROR_FN_EXIT(db, sqlite3_errmsg, "Ошибка доступа к SQLite", exit);
    }

    char query[512];
    // Сначала проверим тип данных в первой строке
    snprintf(query, sizeof(query), "SELECT %s FROM %s LIMIT 1;", col, table);

    if (sqlite3_prepare_v2(db, query, -1, &sql_res, 0) != SQLITE_OK)
    {
        ERROR_FN_EXIT(db, sqlite3_errmsg, "Ошибка подготовки запроса", exit);
    }

    if (sqlite3_step(sql_res) == SQLITE_ROW)
    {
        int type = sqlite3_column_type(sql_res, 0);
        if (type == SQLITE_TEXT || type == SQLITE_BLOB)
        {
            ERROR_EXIT("Ошибка: Колонка [%s] содержит нечисловые данные (TEXT/BLOB).\n", exit, col);
        }
    }
    sqlite3_finalize(sql_res);
    sql_res = NULL;

    // Если проверка пройдена, делаем основной расчет
    snprintf(query, sizeof(query),
             "SELECT AVG(%s), MAX(%s), MIN(%s), SUM(%s), "
             "(SUM(%s*%s) - (SUM(%s)*SUM(%s))/COUNT(%s))/(COUNT(%s)-1) "
             "FROM %s;",
             col, col, col, col, col, col, col, col, col, col, table);

    if (sqlite3_prepare_v2(db, query, -1, &sql_res, 0) != SQLITE_OK)
    {
        ERROR_FN_EXIT(db, sqlite3_errmsg, "Ошибка SQL", exit);
    }

    if (sqlite3_step(sql_res) != SQLITE_ROW)
    {
        ERROR_EXIT("%s\n", exit,"Ошибка: Нет данных для расчёта статистики (таблица пуста или колонка не содержит чисел).");
    }

    if (sqlite3_column_type(sql_res, 0) == SQLITE_NULL)
    {
        ERROR_EXIT("%s\n", exit,"Ошибка: Нет данных для расчёта статистики (таблица пуста или колонка не содержит чисел).");
    }

    stats->avg = sqlite3_column_double(sql_res, 0);
    stats->max = sqlite3_column_double(sql_res, 1);
    stats->min = sqlite3_column_double(sql_res, 2);
    stats->sum = sqlite3_column_double(sql_res, 3);
    stats->var = sqlite3_column_double(sql_res, 4);

exit:
    if (sql_res != NULL)
        sqlite3_finalize(sql_res);
    if (db != NULL)
        sqlite3_close(db);
    return res;
}

// Получение данных для PostgreSQL
int process_postgres(const char *conn_str, const char *table, const char *col, DBStats *stats)
{
    int res = 0;
    PGconn *conn = PQconnectdb(conn_str);
    if (PQstatus(conn) != CONNECTION_OK)
    {
        ERROR_FN_EXIT(conn, PQerrorMessage, "Ошибка подключения", exit);
    }

    char query[512];
    // Попытка приведения типа к double precision вызовет ошибку на стороне сервера, если в данных текст
    snprintf(query, sizeof(query),
             "SELECT AVG(%s::double precision), MAX(%s::double precision), "
             "MIN(%s::double precision), SUM(%s::double precision), "
             "VARIANCE(%s::double precision) FROM %s;",
             col, col, col, col, col, table);

    PGresult *sql_res = PQexec(conn, query);

    if (sql_res == NULL)
    {
        ERROR_FN_EXIT(conn, PQerrorMessage, "Ошибка выполнения запроса", exit);
    }

    if (PQresultStatus(sql_res) != PGRES_TUPLES_OK)
    {
        // Если данные нечисловые, Postgres вернет код ошибки (например, Invalid Text Representation)
        fprintf(stderr, "Ошибка БД: Колонка [%s] имеет несовместимый тип данных или не существует.\n", col);
        ERROR_FN_EXIT(sql_res, PQresultErrorMessage, "Детали", clear);
    }

    if (PQntuples(sql_res) == 0 || PQgetisnull(sql_res, 0, 0))
    {
        ERROR_EXIT("%s\n", exit,"Ошибка: Нет данных для расчёта статистики (таблица пуста или колонка не содержит чисел).");
    }

    stats->avg = atof(PQgetvalue(sql_res, 0, 0));
    stats->max = atof(PQgetvalue(sql_res, 0, 1));
    stats->min = atof(PQgetvalue(sql_res, 0, 2));
    stats->sum = atof(PQgetvalue(sql_res, 0, 3));
    stats->var = PQgetisnull(sql_res, 0, 4) ? 0.0 : atof(PQgetvalue(sql_res, 0, 4));

clear:
    PQclear(sql_res);
exit:
    PQfinish(conn);
    return res;
}

// Проверка: имя должно содержать только [a-zA-Z0-9_] и начинаться с буквы или _
static bool is_valid_identifier(const char *name)
{
    if (name == NULL || *name == '\0')
    {
        goto error;
    }

    // Первый символ: буква или подчёркивание
    if (!isalpha((unsigned char)*name) && *name != '_')
    {
        goto error;
    }

    // Остальные символы: буквы, цифры или подчёркивание
    for (size_t i = 1; name[i] != '\0'; i++)
    {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_')
        {
            goto error;
        }
    }

    return true;
error:
    return false;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Использование: %s <type: sqlite|postgres> <db_source> <table_name> <column_name>\n", argv[0]);
        goto err;
    }

    const char *db_type = argv[1];
    const char *db_source = argv[2];
    const char *table = argv[3];
    const char *column = argv[4];

    // Валидация имён таблицы и колонки
    if (!is_valid_identifier(table))
    {
        fprintf(stderr, "Ошибка: Неверное имя таблицы '%s'. Допустимы только [a-zA-Z0-9_].\n", table);
        goto err;
    }
    if (!is_valid_identifier(column))
    {
        fprintf(stderr, "Ошибка: Неверное имя столбца '%s'. Допустимы только [a-zA-Z0-9_].\n", column);
        goto err;
    }

    DBStats stats = {0};
    int result = -1;

    if (strcmp(db_type, "sqlite") == 0)
    {
        result = process_sqlite(db_source, table, column, &stats);
    }
    else if (strcmp(db_type, "postgres") == 0)
    {
        result = process_postgres(db_source, table, column, &stats);
    }
    else
    {
        fprintf(stderr, "Ошибка: Неподдерживаемый тип БД '%s'. Используйте sqlite или postgres.\n", db_type);
        goto err;
    }

    if (result == 0)
    {
        printf("\n--- Статистика по колонке [%s] ---\n", column);
        printf("Среднее:   %.4f\n", stats.avg);
        printf("Максимум:  %.4f\n", stats.max);
        printf("Минимум:   %.4f\n", stats.min);
        printf("Сумма:     %.4f\n", stats.sum);
        printf("Дисперсия (выборка): %.4f\n", stats.var);
    }

    if (result==0) 
        exit(EXIT_SUCCESS);
err:
    exit(EXIT_FAILURE);
}
