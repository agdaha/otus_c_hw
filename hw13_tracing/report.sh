#!/bin/bash

TRACE_DIR="/sys/kernel/debug/tracing"
TARGET_PROG="./trace_me"

# Сброс настроек
echo 0 > $TRACE_DIR/tracing_on
echo > $TRACE_DIR/set_event_pid
echo > $TRACE_DIR/trace

# Включаем отслеживание системных вызовов
echo 1 > $TRACE_DIR/events/syscalls/enable
echo 1 > $TRACE_DIR/options/event-fork

# Запускаем программу в фоне, чтобы узнать её PID
$TARGET_PROG &
PROG_PID=$!

# Устанавливаем фильтр по PID
echo $PROG_PID > $TRACE_DIR/set_event_pid
echo 1 > $TRACE_DIR/tracing_on

# Ожидаем завершения программы
wait $PROG_PID

# Выключаем запись и сохраняем результат
echo 0 > $TRACE_DIR/tracing_on
cat $TRACE_DIR/trace > result.txt