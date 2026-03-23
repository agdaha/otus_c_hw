#!/bin/bash

# Настройки
SQLITE_DB="test_lite.db"
PG_CONTAINER="test_postgres"
PG_USER="myuser"
PG_PASS="mypass"
PG_DB="testdb"
PG_PORT="5435" # на случай если порт 5432 занят на хостовой машине
TABLE="statistics_test"
COLUMN="val"

# Цвета для вывода
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}>>> Начинаем тестирование...${NC}"

# 1. Подготовка SQLite
echo "--- Настройка SQLite ---"
rm -f $SQLITE_DB
sqlite3 $SQLITE_DB <<EOF
CREATE TABLE $TABLE ($COLUMN REAL, info TEXT);
INSERT INTO $TABLE ($COLUMN, info) VALUES (10.5, 'row1'), (20.0, 'row2'), (30.5, 'row3'), (40.0, 'row4');
EOF

# 2. Подготовка PostgreSQL в Docker
echo "--- Настройка PostgreSQL (Docker) ---"
docker stop $PG_CONTAINER 2>/dev/null || true
docker rm $PG_CONTAINER 2>/dev/null || true

docker run --name $PG_CONTAINER \
    -e POSTGRES_USER=$PG_USER \
    -e POSTGRES_PASSWORD=$PG_PASS \
    -e POSTGRES_DB=$PG_DB \
    -p $PG_PORT:5432 \
    -d postgres:latest > /dev/null

echo "Ожидание запуска Postgres..."
until docker exec $PG_CONTAINER pg_isready -U $PG_USER > /dev/null 2>&1; do
    sleep 1
done

docker exec -i $PG_CONTAINER psql -U $PG_USER -d $PG_DB <<EOF
CREATE TABLE $TABLE ($COLUMN DOUBLE PRECISION, info TEXT);
INSERT INTO $TABLE ($COLUMN, info) VALUES (10.5, 'a'), (20.0, 'b'), (30.5, 'c'), (40.0, 'd');
EOF

# 3. Запуск тестов
echo -e "\n${GREEN}>>> Запуск приложения для SQLite:${NC}"
./stat_db_table sqlite "$SQLITE_DB" "$TABLE" "$COLUMN"

echo -e "\n${GREEN}>>> Запуск приложения для PostgreSQL:${NC}"
PG_CONN="host=localhost port=$PG_PORT dbname=$PG_DB user=$PG_USER password=$PG_PASS"
./stat_db_table postgres "$PG_CONN" "$TABLE" "$COLUMN"

echo -e "\n${GREEN}>>> Тест нечисловой колонки SQLite (должна быть ошибка):${NC}"
./stat_db_table sqlite "$SQLITE_DB" "$TABLE" "info"

echo -e "\n${GREEN}>>> Тест нечисловой колонки PostgreSQL (должна быть ошибка):${NC}"
./stat_db_table postgres "$PG_CONN" "$TABLE" "info"

# 4. Очистка
echo -e "\n--- Очистка ресурсов ---"
rm -f $SQLITE_DB
docker stop $PG_CONTAINER > /dev/null
docker rm $PG_CONTAINER > /dev/null

echo -e "${GREEN}>>> Тестирование завершено.${NC}"